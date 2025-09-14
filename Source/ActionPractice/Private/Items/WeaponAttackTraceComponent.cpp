#include "Items/WeaponAttackTraceComponent.h"
#include "Items/Weapon.h"
#include "Items/WeaponDataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/ActionPracticeCharacter.h"
#include "DrawDebugHelpers.h"
#include "GAS/GameplayTagsSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogWeaponCollision, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

DEFINE_LOG_CATEGORY_STATIC(LogWeaponCollision, Log, All);

UWeaponAttackTraceComponent::UWeaponAttackTraceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UWeaponAttackTraceComponent::BeginPlay()
{    
    OwnerWeapon = Cast<AWeapon>(GetOwner());
    if (!OwnerWeapon)
    {
        DEBUG_LOG(TEXT("WeaponCollisionComponent: Owner is not a weapon!"));
        return;
    }

    SetWeaponStaticMesh();
    
    Super::BeginPlay();
}

void UWeaponAttackTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsTracing || !OwnerWeapon) return;
    
    double TickStart = FPlatformTime::Seconds();
    
    UpdateAdaptiveTraceSettings();
    
    //무기 끝 위치 업데이트 (속도 계산용)
    PrevTipSocketLocation = WeaponStaticMesh->GetSocketTransform(TipSocketName).GetLocation();
    
    TraceAccumulator += DeltaTime;
    
    if (TraceAccumulator >= CurrentSecondsPerTrace)
    {        
        PerformTrace(TraceAccumulator);
        TraceAccumulator = 0.0f;
    }
}

#pragma region "Event Functions"
void UWeaponAttackTraceComponent::BindEventCallbacks()
{
    if (!OwnerWeapon)
    {
        DEBUG_LOG(TEXT("No Owner Weapon"));
        return;
    }
    
    AActionPracticeCharacter* Character = OwnerWeapon->GetOwnerCharacter();
    if (!Character)
    {
        DEBUG_LOG(TEXT("Owner Weapon Has No Character"));
        return;
    }
    
    CachedASC = Character->GetAbilitySystemComponent();
    if (!CachedASC)
    {
        DEBUG_LOG(TEXT("No ASC found on character"));
        return;
    }

    if (HitDetectionStartHandle.IsValid() || HitDetectionEndHandle.IsValid())
    {
        DEBUG_LOG(TEXT("BindEventCallbacks: Clearing previous (potentially orphaned) handles before binding new ones."));
        UnbindEventCallbacks();
    }

    DEBUG_LOG(TEXT("BindEventCallbacks: Subscribing events for %s"), *OwnerWeapon->GetClass()->GetName());
    
    //HitDetectionStart 이벤트 구독
    HitDetectionStartHandle = CachedASC->GenericGameplayEventCallbacks
    .FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyHitDetectionStartTag())
    .AddLambda([this](const FGameplayEventData* EventData)
    {
        if (IsValid(this) && EventData)
        {
            HandleHitDetectionStart(*EventData);
        }
    });
    
    //HitDetectionEnd 이벤트 구독
    HitDetectionEndHandle = CachedASC->GenericGameplayEventCallbacks
    .FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyHitDetectionEndTag())
    .AddLambda([this](const FGameplayEventData* EventData)
    {
        if (IsValid(this) && EventData)
        {
            HandleHitDetectionEnd(*EventData);
        }
    });
}

void UWeaponAttackTraceComponent::UnbindEventCallbacks()
{
    if (CachedASC)
    {
        if (HitDetectionStartHandle.IsValid())
        {
            if (FGameplayEventMulticastDelegate* Delegate = CachedASC->GenericGameplayEventCallbacks.Find(UGameplayTagsSubsystem::GetEventNotifyHitDetectionStartTag()))
            {
                Delegate->Remove(HitDetectionStartHandle);
            }
        }

        if (HitDetectionEndHandle.IsValid())
        {
            if (FGameplayEventMulticastDelegate* Delegate = CachedASC->GenericGameplayEventCallbacks.Find(UGameplayTagsSubsystem::GetEventNotifyHitDetectionEndTag()))
            {
                Delegate->Remove(HitDetectionEndHandle);
            }
        }
    }
    
    HitDetectionStartHandle.Reset();
    HitDetectionEndHandle.Reset();
}

void UWeaponAttackTraceComponent::HandleHitDetectionStart(const FGameplayEventData& Payload)
{
    if (!bIsPrepared)
    {
        DEBUG_LOG(TEXT("HitDetectionStart - Not Prepared"));
        return;
    }
    
    float Duration = Payload.EventMagnitude;
    
    DEBUG_LOG(TEXT("HitDetectionStart Event Received - Duration: %.2f"), Duration);
    
    StartWeaponTrace();
}

void UWeaponAttackTraceComponent::HandleHitDetectionEnd(const FGameplayEventData& Payload)
{
    DEBUG_LOG(TEXT("HitDetectionEnd Event Received"));
    
    StopWeaponTrace();
    bIsPrepared = false;
}
#pragma endregion

#pragma region "Trace Config Functions"
void UWeaponAttackTraceComponent::PrepareHitDetection(const FGameplayTagContainer& AttackTags, const int32 ComboIndex)
{    
    //WeaponData에서 설정 로드
    if (!LoadTraceConfigFromWeaponData(AttackTags, ComboIndex))
    {
        DEBUG_LOG(TEXT("Failed to load trace config for attack tags"));
        return;
    }

    SetWeaponStaticMesh();
    GenerateSocketNames();
    ResetHitActors();
    BindEventCallbacks();
    
    bIsPrepared = true;
    
    DEBUG_LOG(TEXT("PrepareHitDetection - Attack Tags Count: %d, Combo: %d"), AttackTags.Num(), ComboIndex);
}

bool UWeaponAttackTraceComponent::LoadTraceConfigFromWeaponData(const FGameplayTagContainer& AttackTags, int32 ComboIndex)
{
    if (!OwnerWeapon) return false;
    
    const UWeaponDataAsset* WeaponData = OwnerWeapon->GetWeaponData();
    if (!WeaponData) return false;
    
    const FAttackActionData* AttackData = OwnerWeapon->GetWeaponAttackDataByTag(AttackTags);
    if (!AttackData || AttackData->ComboAttackData.Num() == 0) return false;
    
    //콤보 인덱스 유효성 검사
    ComboIndex = FMath::Clamp(ComboIndex, 0, AttackData->ComboAttackData.Num() - 1);
    const FIndividualAttackData& AttackInfo = AttackData->ComboAttackData[ComboIndex];
    
    CurrentConfig.DamageType = AttackInfo.DamageType;
    CurrentConfig.DamageMultiplier = AttackInfo.DamageMultiplier;
    CurrentConfig.StaminaDamage = AttackInfo.StaminaDamage;
    CurrentConfig.SocketCount = WeaponData->SweepTraceSocketCount;
    
    // 고정 반경 사용
    CurrentConfig.TraceRadius = DefaultTraceRadius;
    
    return true;
}

void UWeaponAttackTraceComponent::GenerateSocketNames()
{
    TraceSocketNames.Empty();
    const FString NamePrefix = SocketNamePrefix.ToString();
    
    //trace_socket_0부터 시작, 0이 칼 끝
    for (int32 i = 0; i < CurrentConfig.SocketCount; ++i)
    {
        const FString FullName = FString::Printf(TEXT("%s%d"), *NamePrefix, i);
        
        TraceSocketNames.Add(FName(*FullName));
    }
    
    DEBUG_LOG(TEXT("Generated %d socket names"), TraceSocketNames.Num());
}
#pragma endregion

#pragma region "Trace Functions"
void UWeaponAttackTraceComponent::StartWeaponTrace()
{
    if (bIsTracing)
    {
        DEBUG_LOG(TEXT("Already tracing, stopping previous trace"));
        StopWeaponTrace();
    }
    
    //초기 소켓 위치
    if (!UpdateSocketPositions())
    {
        DEBUG_LOG(TEXT("Failed to update socket positions - no valid sockets found"));
        return;
    }
    
    PrevTipSocketLocation = WeaponStaticMesh->GetSocketTransform(TipSocketName).GetLocation();
    PreviousSocketPositions = CurrentSocketPositions;
    
    TraceAccumulator = 0.0f;
    bIsTracing = true;
    SetComponentTickEnabled(true);

    DebugSweepTraceCounter = 0;
    DEBUG_LOG(TEXT("Started weapon trace"));
}

void UWeaponAttackTraceComponent::StopWeaponTrace()
{
    bIsTracing = false;
    SetComponentTickEnabled(false);
    TraceAccumulator = 0.0f;
    UnbindEventCallbacks();
    WeaponStaticMesh = nullptr;

    DEBUG_LOG(TEXT("Stopped weapon trace, counter: %d"), DebugSweepTraceCounter);
}

void UWeaponAttackTraceComponent::PerformTrace(float DeltaTime)
{
    if (!OwnerWeapon || TraceSocketNames.Num() == 0)
        return;
    
    if (!UpdateSocketPositions())
    {
        DEBUG_LOG(TEXT("cannot update socket positions during trace"));
        StopWeaponTrace();
        return;
    }
    
    switch (CurrentConfig.DamageType)
    {
    case EAttackDamageType::Slash:
        PerformSlashTrace();
        break;
            
    case EAttackDamageType::Pierce:
        PerformPierceTrace();
        break;
            
    case EAttackDamageType::Strike:
        PerformStrikeTrace();
        break;
            
    default:
        DEBUG_LOG(TEXT("Unknown damage type: %d"), (int32)CurrentConfig.DamageType);
        break;
    }
    
    //이전 소켓위치를 현재 소켓위치로 변경
    PreviousSocketPositions = CurrentSocketPositions;
}

bool UWeaponAttackTraceComponent::UpdateSocketPositions()
{
    if (!OwnerWeapon)
        return false;
    
    if (!WeaponStaticMesh)
    {
        return false;        
    }
    
    //소켓을 이름순으로 CurrentSocketPisitions에 저장 (1부터)
    CurrentSocketPositions.Empty();
    for (const FName& SocketName : TraceSocketNames)
    {
        if (WeaponStaticMesh->DoesSocketExist(SocketName))
        {
            FVector SocketLocation = WeaponStaticMesh->GetSocketLocation(SocketName);
            CurrentSocketPositions.Add(SocketLocation);
        }
        else
        {
            DEBUG_LOG(TEXT("Socket %s not found on static mesh"), *SocketName.ToString());
            CurrentSocketPositions.Empty();
            return false;
        }
    }
    return true;
}

void UWeaponAttackTraceComponent::PerformPierceTrace()
{
    
}

void UWeaponAttackTraceComponent::PerformStrikeTrace()
{
    
}

void UWeaponAttackTraceComponent::PerformSlashTrace()
{
    if (CurrentSocketPositions.Num() < 2) return;
    
    TArray<FHitResult> AllHits;
    
    //소켓 이름 순서대로 트레이스 시작과 끝 설정 (0 ~ 1, 1 ~ 2, ...)
    for (int32 i = 0; i < CurrentSocketPositions.Num() - 1; i++)
    {
        FVector StartPrev = PreviousSocketPositions[i];
        FVector StartCurr = CurrentSocketPositions[i];
        
        FVector EndPrev = PreviousSocketPositions[i + 1];
        FVector EndCurr = CurrentSocketPositions[i + 1];
        
        TArray<FHitResult> SubHits;
        
        PerformInterpolationTrace(StartPrev, StartCurr, EndPrev, EndCurr, CurrentConfig.TraceRadius, SubHits);
        AllHits.Append(SubHits);
    }
    
    for (const FHitResult& Hit : AllHits)
    {
        if (ValidateHit(Hit.GetActor(), Hit))
        {
            ProcessHit(Hit.GetActor(), Hit);
        }
    }
}
#pragma endregion

#pragma region "Hit Functions"
bool UWeaponAttackTraceComponent::ValidateHit(AActor* HitActor, const FHitResult& HitResult)
{
    if (!HitActor || !OwnerWeapon)
        return false;
    
    //자기 자신과 소유자 제외
    AActionPracticeCharacter* WeaponOwner = OwnerWeapon->GetOwnerCharacter();
    if (HitActor == OwnerWeapon || HitActor == WeaponOwner) return false;
    
    // 필요 시 아군 제외 구현해야 함
    
    //중복 체크
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (FHitValidationData* ValidationData = HitValidationMap.Find(HitActor))
    {
        if (CurrentTime - ValidationData->LastHitTime < HitCooldownTime)
        {
            DEBUG_LOG(TEXT("Hit rejected: Cooldown (%.2f < %.2f)"), CurrentTime - ValidationData->LastHitTime, HitCooldownTime);
            return false;
        }
        
        //쿨다운 지났으면 업데이트
        ValidationData->LastHitTime = CurrentTime;
        ValidationData->HitCount++;
    }
    else
    {
        //새로운 액터 추가
        FHitValidationData NewData;
        NewData.HitActor = HitActor;
        NewData.LastHitTime = CurrentTime;
        NewData.HitCount = 1;
        HitValidationMap.Add(HitActor, NewData);
    }
    
    return true;
}

void UWeaponAttackTraceComponent::ProcessHit(AActor* HitActor, const FHitResult& HitResult)
{
    float FinalMultiplier = CurrentConfig.DamageMultiplier;
    
    DEBUG_LOG(TEXT("Hit %s - Type: %d, DmgMult: %.2f"), *HitActor->GetName(), (int32)CurrentConfig.DamageType, FinalMultiplier);
    
    //온힛 이벤트
    OnWeaponHit.Broadcast(HitActor, HitResult, CurrentConfig.DamageType, FinalMultiplier);
}

void UWeaponAttackTraceComponent::ResetHitActors()
{
    HitValidationMap.Empty();
    DEBUG_LOG(TEXT("Reset hit actors"));
}

#pragma endregion

#pragma region "Adaptive Trace Functions"
float UWeaponAttackTraceComponent::CalculateSwingSpeed() const
{
    if (!OwnerWeapon || !WeaponStaticMesh || !GetWorld()) return 0.0f;

    //속도 = 거리 / 시간이므로 DeltaTime이 필요합니다.
    const float DeltaTime = GetWorld()->GetDeltaSeconds();
    if (DeltaTime <= KINDA_SMALL_NUMBER) //0으로 나누기 방지
    {
        return 0.0f;
    }

    FVector CurTipSocketLocation = WeaponStaticMesh->GetSocketTransform(TipSocketName).GetLocation();

    //속도 (cm/s) 계산
    const float DistanceTraveled = FVector::Dist(CurTipSocketLocation, PrevTipSocketLocation);
    const float SpeedInCmPerSecond = DistanceTraveled / DeltaTime;

    return SpeedInCmPerSecond;
}

void UWeaponAttackTraceComponent::UpdateAdaptiveTraceSettings()
{
    float SwingSpeed = CalculateSwingSpeed();
    
    //속도에 따른 설정 선택
    FAdaptiveTraceConfig SelectedConfig = AdaptiveConfigs[0];
    int32 i = 0;
    for (i; i < AdaptiveConfigs.Num(); ++i)
    {
        if (SwingSpeed >= AdaptiveConfigs[i].SpeedThreshold)
        {
            SelectedConfig = AdaptiveConfigs[i];
        }
        else
        {
            break;
        }
    }
    
    CurrentSecondsPerTrace = SelectedConfig.SecondsPerTrace;
    CurrentInterpolationPerTrace = SelectedConfig.InterpolationPerTrace;
    
    DEBUG_LOG(TEXT("Adaptive Trace - Speed: %.1f, Interval: %.3f, Subdivisions: %d, level: %d"), SwingSpeed, CurrentSecondsPerTrace, CurrentInterpolationPerTrace, i - 1);
}

void UWeaponAttackTraceComponent::PerformInterpolationTrace(
    const FVector& StartPrev, const FVector& StartCurr,
    const FVector& EndPrev, const FVector& EndCurr,
    float Radius, TArray<FHitResult>& OutHits)
{
    //이전 프레임 소켓 위치와 현재 프레임 소켓 위치 사이의 간극이 큰 것을 방지하기 위해
    //보간으로 프레임 간 중간 지점을 찾아 스윕 포인트 추가
    
    const int32 InterpolationPerTrace = CurrentInterpolationPerTrace;  //보간 세분화 수
    FCollisionQueryParams Params = GetCollisionQueryParams();
    
    for (int32 i = 0; i <= InterpolationPerTrace; ++i)
    {
        float Alpha = (float)i / (float)InterpolationPerTrace;
        
        FVector InterpStart = FMath::Lerp(StartPrev, StartCurr, Alpha);
        FVector InterpEnd = FMath::Lerp(EndPrev, EndCurr, Alpha);
        
        TArray<FHitResult> SubHits;
        
        GetWorld()->SweepMultiByChannel(
            SubHits,
            InterpStart,
            InterpEnd,
            FQuat::Identity,
            GetTraceChannel(),
            FCollisionShape::MakeCapsule(Radius, (InterpEnd - InterpStart).Size() * 0.5f),
            Params
        );
        
        OutHits.Append(SubHits);

        ++DebugSweepTraceCounter;
        if (bDrawDebugTrace)
        {
            DrawDebugCapsule(GetWorld(), 
                           (InterpStart + InterpEnd) * 0.5f,
                           (InterpEnd - InterpStart).Size() * 0.5f,
                           Radius,
                           FQuat::FindBetweenNormals(FVector::UpVector, (InterpEnd - InterpStart).GetSafeNormal()),
                           DebugTraceColor,
                           false,
                           DebugTraceDuration);
        }
    }
}
#pragma endregion

#pragma region "Utility Functions"
void UWeaponAttackTraceComponent::SetWeaponStaticMesh()
{
    WeaponStaticMesh = OwnerWeapon->FindComponentByClass<UStaticMeshComponent>();
    //무기 끝 소켓까지 확인
    if (!WeaponStaticMesh || !WeaponStaticMesh->DoesSocketExist(FName(*FString::Printf(TEXT("trace_socket_0")))) )
    {
        WeaponStaticMesh = nullptr;
        DEBUG_LOG(TEXT("WeaponCollisionComponent: No Weapon StaticMesh or trace_socket_0"));
        return;
    }
    
    TipSocketName = FName(*FString::Printf(TEXT("%s0"), *SocketNamePrefix.ToString()));
}

ECollisionChannel UWeaponAttackTraceComponent::GetTraceChannel() const
{
    //프로젝트 설정에서 커스텀 채널 사용
    // DefaultEngine.ini에서 설정 필요
    return ECC_GameTraceChannel1;  // WeaponTrace 채널
}

FCollisionQueryParams UWeaponAttackTraceComponent::GetCollisionQueryParams() const
{
    FCollisionQueryParams Params(TEXT("WeaponTrace"), false);
    
    if (OwnerWeapon)
    {
        Params.AddIgnoredActor(OwnerWeapon);
        
        if (AActionPracticeCharacter* WeaponOwner = OwnerWeapon->GetOwnerCharacter())
        {
            Params.AddIgnoredActor(WeaponOwner);
        }
    }
    
    return Params;
}
#pragma endregion

void UWeaponAttackTraceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnbindEventCallbacks();
    
    Super::EndPlay(EndPlayReason);
}

#pragma region "Debug And Profiling"
void UWeaponAttackTraceComponent::DrawDebugSweepTrace(
    const FVector& StartPrev, const FVector& StartCurr,
    const FVector& EndPrev, const FVector& EndCurr,
    float Radius, const FColor& Color)
{
    //사다리꼴 와이어프레임 그리기
    DrawDebugLine(GetWorld(), StartPrev, EndPrev, Color, false, DebugTraceDuration);
    DrawDebugLine(GetWorld(), StartCurr, EndCurr, Color, false, DebugTraceDuration);
    DrawDebugLine(GetWorld(), StartPrev, StartCurr, Color, false, DebugTraceDuration);
    DrawDebugLine(GetWorld(), EndPrev, EndCurr, Color, false, DebugTraceDuration);
    
    //모서리에 구체 그리기
    DrawDebugSphere(GetWorld(), StartPrev, Radius, 8, Color, false, DebugTraceDuration);
    DrawDebugSphere(GetWorld(), StartCurr, Radius, 8, Color, false, DebugTraceDuration);
    DrawDebugSphere(GetWorld(), EndPrev, Radius, 8, Color, false, DebugTraceDuration);
    DrawDebugSphere(GetWorld(), EndCurr, Radius, 8, Color, false, DebugTraceDuration);
}
#pragma endregion