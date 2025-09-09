#include "Items/WeaponCollisionComponent.h"
#include "Items/Weapon.h"
#include "Items/WeaponData.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/ActionPracticeCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"
#include "GAS/GameplayTagsSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogWeaponCollision, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

DEFINE_LOG_CATEGORY_STATIC(LogWeaponCollision, Log, All);

UWeaponCollisionComponent::UWeaponCollisionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UWeaponCollisionComponent::BeginPlay()
{    
    OwnerWeapon = Cast<AWeapon>(GetOwner());
    if (!OwnerWeapon)
    {
        DEBUG_LOG(TEXT("WeaponCollisionComponent: Owner is not a weapon!"));
        return;
    }

    Super::BeginPlay();
    
    //이벤트 리스너 설정
    BindEventCallbacks();
}

void UWeaponCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsTracing || !OwnerWeapon)
        return;
    
    //적응형 트레이스 레이트 계산
    float AdaptiveRate = CalculateAdaptiveTraceRate();
    CurrentTraceInterval = 1.0f / AdaptiveRate;
    
    //트레이스 타이머 업데이트
    TraceAccumulator += DeltaTime;
    ElapsedTraceTime += DeltaTime;
    
    if (TraceAccumulator >= CurrentTraceInterval)
    {
        PerformTrace(TraceAccumulator);
        TraceAccumulator = 0.0f;
    }
    
    // 무기 위치/회전 업데이트 (속도 계산용)
    PreviousWeaponLocation = OwnerWeapon->GetActorLocation();
    PreviousWeaponRotation = OwnerWeapon->GetActorQuat();
}

#pragma region "Event Functions"
void UWeaponCollisionComponent::BindEventCallbacks()
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

void UWeaponCollisionComponent::UnbindEventCallbacks()
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
}

void UWeaponCollisionComponent::HandleHitDetectionStart(const FGameplayEventData& Payload)
{
    if (!bIsPrepared)
    {
        DEBUG_LOG(TEXT("HitDetectionStart - Not Prepared"));
        return;
    }
    
    float Duration = Payload.EventMagnitude;
    
    DEBUG_LOG(TEXT("HitDetectionStart Event Received - Duration: %.2f"), Duration);
    
    StartWeaponTrace();
    TotalTraceDuration = Duration;
}

void UWeaponCollisionComponent::HandleHitDetectionEnd(const FGameplayEventData& Payload)
{
    DEBUG_LOG(TEXT("HitDetectionEnd Event Received"));
    
    StopWeaponTrace();
    bIsPrepared = false;
}
#pragma endregion

#pragma region "Trace Setting Functions"
void UWeaponCollisionComponent::PrepareHitDetection(const FGameplayTag& AttackTag, const int32 ComboIndex)
{
    CurrentAttackTag = AttackTag;
    CurrentComboIndex = ComboIndex;
    
    //WeaponData에서 설정 로드
    if (!LoadTraceConfigFromWeaponData(AttackTag, ComboIndex))
    {
        DEBUG_LOG(TEXT("Failed to load trace config for attack: %s"), *AttackTag.ToString());
        return;
    }
    
    GenerateSocketNames();
    ResetHitActors();
    
    bIsPrepared = true;
    
    DEBUG_LOG(TEXT("PrepareHitDetection - Attack: %s, Combo: %d"), *AttackTag.ToString(), ComboIndex);
}

bool UWeaponCollisionComponent::LoadTraceConfigFromWeaponData(const FGameplayTag& AttackTag, int32 ComboIndex)
{
    if (!OwnerWeapon) return false;
    
    const UWeaponDataAsset* WeaponData = OwnerWeapon->GetWeaponData();
    if (!WeaponData) return false;
    
    const FAttackActionData* AttackData = OwnerWeapon->GetWeaponAttackDataByTag(AttackTag);
    if (!AttackData || AttackData->ComboAttackData.Num() == 0) return false;
    
    //콤보 인덱스 유효성 검사
    ComboIndex = FMath::Clamp(ComboIndex, 0, AttackData->ComboAttackData.Num() - 1);
    const FIndividualAttackData& AttackInfo = AttackData->ComboAttackData[ComboIndex];
    
    CurrentConfig.DamageType = AttackInfo.DamageType;
    CurrentConfig.DamageMultiplier = AttackInfo.DamageMultiplier;
    CurrentConfig.StaminaCost = AttackInfo.StaminaCost;
    CurrentConfig.StaminaDamage = AttackInfo.StaminaDamage;
    
    CurrentConfig.SocketCount = WeaponData->SweepTraceSocketCount;
    
    //공격 타입별 기본 트레이스 반경 설정
    switch (CurrentConfig.DamageType)
    {
    case EAttackDamageType::Slash:
        CurrentConfig.TraceRadius = DefaultTraceRadius;
        break;
            
    case EAttackDamageType::Pierce:
        CurrentConfig.TraceRadius = DefaultTraceRadius * 0.7f;  // 찌르기는 좁게
        break;
            
    case EAttackDamageType::Strike:
        CurrentConfig.TraceRadius = DefaultTraceRadius * 1.5f;  // 타격은 넓게
        break;
            
    default:
        CurrentConfig.TraceRadius = DefaultTraceRadius;
        break;
    }
    
    return true;
}

void UWeaponCollisionComponent::GenerateSocketNames()
{
    TraceSocketNames.Empty();
    
    //trace_socket_1, trace_socket_2, ... 형식으로 소켓 가져옴
    for (int32 i = 1; i <= CurrentConfig.SocketCount; i++)
    {
        FName SocketName = FName(*FString::Printf(TEXT("trace_socket_%d"), i));
        TraceSocketNames.Add(SocketName);
    }
    
    DEBUG_LOG(TEXT("Generated %d socket names"), TraceSocketNames.Num());
}
#pragma endregion

#pragma region "Trace Functions"
void UWeaponCollisionComponent::StartWeaponTrace()
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
    
    SweptFrame.PreviousSocketPositions = SweptFrame.CurrentSocketPositions;
    
    ElapsedTraceTime = 0.0f;
    TraceAccumulator = 0.0f;
    bIsTracing = true;
    SetComponentTickEnabled(true);
    
    DEBUG_LOG(TEXT("Started weapon trace"));
}

void UWeaponCollisionComponent::StopWeaponTrace()
{
    bIsTracing = false;
    SetComponentTickEnabled(false);
    TraceAccumulator = 0.0f;
    
    DEBUG_LOG(TEXT("Stopped weapon trace"));
}

void UWeaponCollisionComponent::PerformTrace(float DeltaTime)
{
    if (!OwnerWeapon || TraceSocketNames.Num() == 0)
        return;
    
    if (!UpdateSocketPositions())
    {
        DEBUG_LOG(TEXT("Failed to update socket positions during trace"));
        StopWeaponTrace();
        return;
    }
    
    //스윕 프레임 설정
    SweptFrame.DeltaTime = DeltaTime;
    
    switch (CurrentConfig.DamageType)
    {
    case EAttackDamageType::Slash:
        PerformSlashTrace(SweptFrame);
        break;
            
    case EAttackDamageType::Pierce:
        PerformPierceTrace(SweptFrame);
        break;
            
    case EAttackDamageType::Strike:
        PerformStrikeTrace(SweptFrame);
        break;
            
    default:
        DEBUG_LOG(TEXT("Unknown damage type: %d"), (int32)CurrentConfig.DamageType);
        break;
    }
    
    //위치 변경
    SweptFrame.PreviousSocketPositions = SweptFrame.CurrentSocketPositions;
}

bool UWeaponCollisionComponent::UpdateSocketPositions()
{
    if (!OwnerWeapon)
        return false;
    
    USkeletalMeshComponent* WeaponMesh = OwnerWeapon->FindComponentByClass<USkeletalMeshComponent>();
    if (!WeaponMesh)
    {
        //스태틱 메시인 경우
        UStaticMeshComponent* StaticWeaponMesh = OwnerWeapon->FindComponentByClass<UStaticMeshComponent>();
        if (!StaticWeaponMesh) return false;
            
        //소켓 체크
        SweptFrame.CurrentSocketPositions.Empty();
        for (const FName& SocketName : TraceSocketNames)
        {
            if (StaticWeaponMesh->DoesSocketExist(SocketName))
            {
                FVector SocketLocation = StaticWeaponMesh->GetSocketLocation(SocketName);
                SweptFrame.CurrentSocketPositions.Add(SocketLocation);
            }
            else
            {
                DEBUG_LOG(TEXT("Socket %s not found on static mesh"), *SocketName.ToString());
                SweptFrame.CurrentSocketPositions.Empty();
                return false;
            }
        }
        return true;
    }
    
    //스켈레탈 메시인 경우
    SweptFrame.CurrentSocketPositions.Empty();
    for (const FName& SocketName : TraceSocketNames)
    {
        if (WeaponMesh->DoesSocketExist(SocketName))
        {
            FVector SocketLocation = WeaponMesh->GetSocketLocation(SocketName);
            SweptFrame.CurrentSocketPositions.Add(SocketLocation);
        }
        else
        {
            DEBUG_LOG(TEXT("Socket %s not found on skeletal mesh"), *SocketName.ToString());
            SweptFrame.CurrentSocketPositions.Empty();
            return false;
        }
    }
    
    return true;
}

void UWeaponCollisionComponent::PerformSlashTrace(const FSweptFrame& Frame)
{
    // 참격: 모든 소켓을 연결하는 캡슐 스윕
    if (Frame.CurrentSocketPositions.Num() < 2)
        return;
    
    TArray<FHitResult> AllHits;
    
    // 인접한 소켓 쌍으로 스윕
    for (int32 i = 0; i < Frame.CurrentSocketPositions.Num() - 1; i++)
    {
        FVector StartPrev = Frame.PreviousSocketPositions.IsValidIndex(i) ? 
                           Frame.PreviousSocketPositions[i] : Frame.CurrentSocketPositions[i];
        FVector StartCurr = Frame.CurrentSocketPositions[i];
        
        FVector EndPrev = Frame.PreviousSocketPositions.IsValidIndex(i + 1) ? 
                         Frame.PreviousSocketPositions[i + 1] : Frame.CurrentSocketPositions[i + 1];
        FVector EndCurr = Frame.CurrentSocketPositions[i + 1];
        
        TArray<FHitResult> SegmentHits;
        if (CreateSweptVolume(StartPrev, StartCurr, EndPrev, EndCurr, 
                            CurrentConfig.TraceRadius, SegmentHits))
        {
            AllHits.Append(SegmentHits);
        }
    }
    
    for (const FHitResult& Hit : AllHits)
    {
        if (ValidateHit(Hit.GetActor(), Hit))
        {
            ProcessHit(Hit.GetActor(), Hit);
        }
    }
}

void UWeaponCollisionComponent::PerformPierceTrace(const FSweptFrame& Frame)
{
    // 관통: 마지막 소켓(끝)에서 라인 트레이스
    if (Frame.CurrentSocketPositions.Num() == 0)
        return;
    
    // 끝 소켓 위치
    FVector TipPrev = Frame.PreviousSocketPositions.IsValidIndex(Frame.PreviousSocketPositions.Num() - 1) ?
                     Frame.PreviousSocketPositions.Last() : Frame.CurrentSocketPositions.Last();
    FVector TipCurr = Frame.CurrentSocketPositions.Last();
    
    // 관통 방향으로 확장
    FVector PierceDirection = (TipCurr - TipPrev).GetSafeNormal();
    if (PierceDirection.IsNearlyZero())
    {
        PierceDirection = OwnerWeapon->GetActorForwardVector();
    }
    FVector ExtendedEnd = TipCurr + PierceDirection * CurrentConfig.TraceRadius * 3.0f;
    
    TArray<FHitResult> Hits;
    FCollisionQueryParams Params = GetCollisionQueryParams();
    
    GetWorld()->LineTraceMultiByChannel(
        Hits,
        TipPrev,
        ExtendedEnd,
        GetTraceChannel(),
        Params
    );
    
    if (bDrawDebugTrace)
    {
        DrawDebugLine(GetWorld(), TipPrev, ExtendedEnd, FColor::Yellow, false, DebugTraceDuration, 0, 2.0f);
    }
    
    for (const FHitResult& Hit : Hits)
    {
        if (ValidateHit(Hit.GetActor(), Hit))
        {
            ProcessHit(Hit.GetActor(), Hit);
        }
    }
}

void UWeaponCollisionComponent::PerformStrikeTrace(const FSweptFrame& Frame)
{
    // 타격: 중간 지점에서 구체 스윕
    if (Frame.CurrentSocketPositions.Num() == 0)
        return;
    
    // 중간 지점 계산
    FVector ImpactPrev, ImpactCurr;
    
    if (Frame.CurrentSocketPositions.Num() == 1)
    {
        // 소켓이 하나면 그 위치 사용
        ImpactPrev = Frame.PreviousSocketPositions.IsValidIndex(0) ? 
                    Frame.PreviousSocketPositions[0] : Frame.CurrentSocketPositions[0];
        ImpactCurr = Frame.CurrentSocketPositions[0];
    }
    else
    {
        // 여러 소켓의 중심점 계산
        int32 MiddleIndex = Frame.CurrentSocketPositions.Num() / 2;
        ImpactPrev = Frame.PreviousSocketPositions.IsValidIndex(MiddleIndex) ? 
                    Frame.PreviousSocketPositions[MiddleIndex] : Frame.CurrentSocketPositions[MiddleIndex];
        ImpactCurr = Frame.CurrentSocketPositions[MiddleIndex];
    }
    
    TArray<FHitResult> Hits;
    FCollisionQueryParams Params = GetCollisionQueryParams();
    
    // 큰 반경의 구체 스윕
    float ImpactRadius = CurrentConfig.TraceRadius;
    
    GetWorld()->SweepMultiByChannel(
        Hits,
        ImpactPrev,
        ImpactCurr,
        FQuat::Identity,
        GetTraceChannel(),
        FCollisionShape::MakeSphere(ImpactRadius),
        Params
    );
    
    if (bDrawDebugTrace)
    {
        DrawDebugSphere(GetWorld(), ImpactCurr, ImpactRadius, 
                       12, FColor::Red, false, DebugTraceDuration);
    }
    
    for (const FHitResult& Hit : Hits)
    {
        if (ValidateHit(Hit.GetActor(), Hit))
        {
            ProcessHit(Hit.GetActor(), Hit);
        }
    }
}

bool UWeaponCollisionComponent::CreateSweptVolume(const FVector& StartPrev, const FVector& StartCurr,
                                                 const FVector& EndPrev, const FVector& EndCurr,
                                                 float Radius, TArray<FHitResult>& OutHits)
{
    // 사다리꼴 형태의 스윕 볼륨을 여러 캡슐로 근사
    const int32 NumSubdivisions = 3;  // 보간 세분화 수
    
    FCollisionQueryParams Params = GetCollisionQueryParams();
    bool bHitDetected = false;
    
    for (int32 i = 0; i <= NumSubdivisions; i++)
    {
        float Alpha = (float)i / (float)NumSubdivisions;
        
        // 선형 보간으로 중간 위치 계산
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
        
        if (SubHits.Num() > 0)
        {
            OutHits.Append(SubHits);
            bHitDetected = true;
        }
        
        if (bDrawDebugTrace)
        {
            DrawDebugCapsule(GetWorld(), 
                           (InterpStart + InterpEnd) * 0.5f,
                           (InterpEnd - InterpStart).Size() * 0.5f,
                           Radius,
                           FQuat::FindBetweenNormals(FVector::UpVector, (InterpEnd - InterpStart).GetSafeNormal()),
                           FColor::Green,
                           false,
                           DebugTraceDuration);
        }
    }
    
    return bHitDetected;
}
#pragma endregion

#pragma region "Hit Functions"
bool UWeaponCollisionComponent::ValidateHit(AActor* HitActor, const FHitResult& HitResult)
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

void UWeaponCollisionComponent::ProcessHit(AActor* HitActor, const FHitResult& HitResult)
{
    float FinalMultiplier = CurrentConfig.DamageMultiplier;
    
    DEBUG_LOG(TEXT("Hit %s - Type: %d, DmgMult: %.2f"), *HitActor->GetName(), (int32)CurrentConfig.DamageType, FinalMultiplier);
    
    //온힛 이벤트
    OnWeaponHit.Broadcast(HitActor, HitResult, CurrentConfig.DamageType, FinalMultiplier);
}

void UWeaponCollisionComponent::ResetHitActors()
{
    HitValidationMap.Empty();
    DEBUG_LOG(TEXT("Reset hit actors"));
}
#pragma endregion

float UWeaponCollisionComponent::CalculateSwingSpeed() const
{
    if (!OwnerWeapon)
        return 0.0f;
    
    //선속도 계산
    FVector CurrentLocation = OwnerWeapon->GetActorLocation();
    float LinearSpeed = (CurrentLocation - PreviousWeaponLocation).Size();
    
    //각속도 계산
    FQuat CurrentRotation = OwnerWeapon->GetActorQuat();
    float AngleDelta = CurrentRotation.AngularDistance(PreviousWeaponRotation);
    
    //무기 길이 추정 (소켓 간 거리)
    float WeaponLength = 100.0f;  //기본값
    if (TraceSocketNames.Num() >= 2)
    {
        USkeletalMeshComponent* WeaponMesh = OwnerWeapon->FindComponentByClass<USkeletalMeshComponent>();
        if (!WeaponMesh)
        {
            UStaticMeshComponent* StaticWeaponMesh = OwnerWeapon->FindComponentByClass<UStaticMeshComponent>();
            if (StaticWeaponMesh && StaticWeaponMesh->DoesSocketExist(TraceSocketNames[0]) && 
                StaticWeaponMesh->DoesSocketExist(TraceSocketNames.Last()))
            {
                FVector Start = StaticWeaponMesh->GetSocketLocation(TraceSocketNames[0]);
                FVector End = StaticWeaponMesh->GetSocketLocation(TraceSocketNames.Last());
                WeaponLength = (End - Start).Size();
            }
        }
        else if (WeaponMesh->DoesSocketExist(TraceSocketNames[0]) && 
                 WeaponMesh->DoesSocketExist(TraceSocketNames.Last()))
        {
            FVector Start = WeaponMesh->GetSocketLocation(TraceSocketNames[0]);
            FVector End = WeaponMesh->GetSocketLocation(TraceSocketNames.Last());
            WeaponLength = (End - Start).Size();
        }
    }
    
    float AngularSpeed = AngleDelta * WeaponLength;
    
    return LinearSpeed + AngularSpeed;
}

float UWeaponCollisionComponent::CalculateAdaptiveTraceRate() const
{
    float SwingSpeed = CalculateSwingSpeed();
    
    //속도에 따라 트레이스 레이트 조정
    float AdaptiveRate = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, SwingSpeedThreshold),
        FVector2D(BaseTraceRate, MaxTraceRate),
        SwingSpeed
    );
    
    return AdaptiveRate;
}

void UWeaponCollisionComponent::DrawDebugSweptVolume(const FVector& StartPrev, const FVector& StartCurr, const FVector& EndPrev,
                                                        const FVector& EndCurr, float Radius, const FColor& Color)
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

ECollisionChannel UWeaponCollisionComponent::GetTraceChannel() const
{
    //프로젝트 설정에서 커스텀 채널 사용
    // DefaultEngine.ini에서 설정 필요
    return ECC_GameTraceChannel1;  // WeaponTrace 채널
}

FCollisionQueryParams UWeaponCollisionComponent::GetCollisionQueryParams() const
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

void UWeaponCollisionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnbindEventCallbacks();
    
    Super::EndPlay(EndPlayReason);
}