#include "Items/WeaponCollisionComponent.h"
#include "Items/Weapon.h"
#include "Items/WeaponData.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

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
    Super::BeginPlay();
    
    OwnerWeapon = Cast<AWeapon>(GetOwner());
    if (!OwnerWeapon)
    {
        DEBUG_LOG(TEXT("WeaponCollisionComponent: Owner is not a weapon!"));
    }
}

void UWeaponCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsTracing || !OwnerWeapon)
        return;
    
    // 적응형 트레이스 레이트 계산
    float AdaptiveRate = CalculateAdaptiveTraceRate();
    CurrentTraceInterval = 1.0f / AdaptiveRate;
    
    // 트레이스 타이머 업데이트
    TraceAccumulator += DeltaTime;
    
    if (TraceAccumulator >= CurrentTraceInterval)
    {
        PerformTrace(TraceAccumulator);
        TraceAccumulator = 0.0f;
    }
    
    // 무기 위치/회전 업데이트 (속도 계산용)
    PreviousWeaponLocation = OwnerWeapon->GetActorLocation();
    PreviousWeaponRotation = OwnerWeapon->GetActorQuat();
}

void UWeaponCollisionComponent::StartWeaponTrace(const FGameplayTag& AttackTag, float Duration)
{
    if (bIsTracing)
    {
        DEBUG_LOG(TEXT("Already tracing, stopping previous trace"));
        StopWeaponTrace();
    }
    
    CurrentAttackTag = AttackTag;
    TotalTraceDuration = Duration;
    ElapsedTraceTime = 0.0f;
    
    // WeaponData에서 설정 로드
    if (!LoadTraceConfigFromWeaponData(AttackTag))
    {
        DEBUG_LOG(TEXT("Failed to load trace config for attack: %s"), *AttackTag.ToString());
        return;
    }
    
    // 소켓 이름 자동 생성
    GenerateSocketNames();
    
    // 초기 소켓 위치 저장
    UpdateSocketPositions();
    SweptFrame.PreviousSocketPositions = SweptFrame.CurrentSocketPositions;
    
    bIsTracing = true;
    SetComponentTickEnabled(true);
    
    DEBUG_LOG(TEXT("Started weapon trace - Attack: %s, DamageType: %d, Sockets: %d"),
             *AttackTag.ToString(), (int32)CurrentConfig.DamageType, CurrentConfig.SocketCount);
}

void UWeaponCollisionComponent::StopWeaponTrace()
{
    bIsTracing = false;
    SetComponentTickEnabled(false);
    TraceAccumulator = 0.0f;
    
    DEBUG_LOG(TEXT("Stopped weapon trace"));
}

bool UWeaponCollisionComponent::LoadTraceConfigFromWeaponData(const FGameplayTag& AttackTag)
{
    if (!OwnerWeapon)
        return false;
    
    const UWeaponDataAsset* WeaponData = OwnerWeapon->GetWeaponData();
    if (!WeaponData)
        return false;
    
    // AttackTag에 해당하는 공격 데이터 찾기
    const FAttackActionData* AttackData = WeaponData->AttackDataMap.Find(AttackTag);
    if (!AttackData || AttackData->ComboAttackData.Num() == 0)
        return false;
    
    // 현재 콤보 인덱스에 해당하는 데이터 가져오기
    // TODO: 콤보 시스템과 연동하여 CurrentComboIndex 설정
    CurrentComboIndex = FMath::Clamp(CurrentComboIndex, 0, AttackData->ComboAttackData.Num() - 1);
    const FIndividualAttackData& AttackInfo = AttackData->ComboAttackData[CurrentComboIndex];
    
    // 설정 로드
    CurrentConfig.DamageType = AttackInfo.DamageType;
    CurrentConfig.DamageMultiplier = AttackInfo.DamageMultiplier;
    CurrentConfig.StaminaCost = AttackInfo.StaminaCost;
    CurrentConfig.StaminaDamage = AttackInfo.StaminaDamage;
    
    // WeaponData에서 소켓 개수 가져오기
    CurrentConfig.SocketCount = WeaponData->SweepTraceSocketCount;
    
    // 공격 타입별 기본 트레이스 반경 설정
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

void UWeaponCollisionComponent::SetTraceProfile(const FWeaponTraceProfile& NewProfile)
{
    CurrentProfile = NewProfile;
    DEBUG_LOG(TEXT("Set trace profile with %d sockets"), NewProfile.SocketNames.Num());
}

void UWeaponCollisionComponent::ResetHitActors()
{
    HitValidationMap.Empty();
    DEBUG_LOG(TEXT("Reset hit actors"));
}

void UWeaponCollisionComponent::GenerateSocketNames()
{
    TraceSocketNames.Empty();
    
    // trace_socket_1, trace_socket_2, ... 형식으로 생성
    for (int32 i = 1; i <= CurrentConfig.SocketCount; i++)
    {
        FName SocketName = FName(*FString::Printf(TEXT("trace_socket_%d"), i));
        TraceSocketNames.Add(SocketName);
    }
    
    DEBUG_LOG(TEXT("Generated %d socket names"), TraceSocketNames.Num());
}

void UWeaponCollisionComponent::UpdateSocketPositions()
{
    if (!OwnerWeapon)
        return;
    
    USkeletalMeshComponent* WeaponMesh = OwnerWeapon->FindComponentByClass<USkeletalMeshComponent>();
    if (!WeaponMesh)
        return;
    
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
            // 소켓이 없으면 무기 위치 기준으로 추정
            FVector WeaponLocation = OwnerWeapon->GetActorLocation();
            FVector WeaponForward = OwnerWeapon->GetActorForwardVector();
            
            // 소켓 인덱스에 따라 위치 분배
            float Alpha = (TraceSocketNames.Find(SocketName) + 1.0f) / (TraceSocketNames.Num() + 1.0f);
            FVector EstimatedLocation = WeaponLocation + WeaponForward * (Alpha * 100.0f);
            
            SweptFrame.CurrentSocketPositions.Add(EstimatedLocation);
            
            DEBUG_LOG(TEXT("Socket %s not found, using estimated position"), *SocketName.ToString());
        }
    }
}

void UWeaponCollisionComponent::PerformTrace(float DeltaTime)
{
    if (!OwnerWeapon || TraceSocketNames.Num() == 0)
        return;
    
    // 현재 소켓 위치 업데이트
    UpdateSocketPositions();
    
    // 스윕 프레임 설정
    SweptFrame.DeltaTime = DeltaTime;
    
    // 공격 유형에 따른 트레이스 수행
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
    
    // 현재 위치를 이전 위치로 저장
    SweptFrame.PreviousSocketPositions = SweptFrame.CurrentSocketPositions;
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
    
    // 히트 처리
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
        DrawDebugLine(GetWorld(), TipPrev, ExtendedEnd, 
                     FColor::Yellow, false, DebugTraceDuration, 0, 2.0f);
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

void UWeaponCollisionComponent::ProcessHit(AActor* HitActor, const FHitResult& HitResult)
{
    // 기본 데미지 배율
    float FinalMultiplier = CurrentConfig.DamageMultiplier;
    
    // 속도 기반 데미지 보정
    float SwingSpeed = CalculateSwingSpeed();
    float SpeedMultiplier = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, SwingSpeedThreshold),
        FVector2D(0.5f, 1.5f),
        SwingSpeed
    );
    
    FinalMultiplier *= SpeedMultiplier;
    
    DEBUG_LOG(TEXT("Hit %s - Type: %d, DmgMult: %.2f, SpeedMult: %.2f, Final: %.2f"),
             *HitActor->GetName(), (int32)CurrentConfig.DamageType, 
             CurrentConfig.DamageMultiplier, SpeedMultiplier, FinalMultiplier);
    
    // 델리게이트 브로드캐스트
    OnWeaponHit.Broadcast(HitActor, HitResult, CurrentConfig.DamageType, FinalMultiplier);
}

void UWeaponCollisionComponent::RegisterGameplayEventCallbacks()
{
    
}

void UWeaponCollisionComponent::UnRegisterGameplayEventCallbacks()
{
    
}

float UWeaponCollisionComponent::CalculateSwingSpeed() const
{
    if (!OwnerWeapon)
        return 0.0f;
    
    // 선속도 계산
    FVector CurrentLocation = OwnerWeapon->GetActorLocation();
    float LinearSpeed = (CurrentLocation - PreviousWeaponLocation).Size();
    
    // 각속도 계산
    FQuat CurrentRotation = OwnerWeapon->GetActorQuat();
    float AngleDelta = CurrentRotation.AngularDistance(PreviousWeaponRotation);
    
    // 무기 길이 추정 (소켓 간 거리)
    float WeaponLength = 100.0f;  // 기본값
    if (CurrentProfile.SocketNames.Num() >= 2)
    {
        USkeletalMeshComponent* WeaponMesh = OwnerWeapon->FindComponentByClass<USkeletalMeshComponent>();
        if (WeaponMesh)
        {
            FVector Start = WeaponMesh->GetSocketLocation(CurrentProfile.SocketNames[0]);
            FVector End = WeaponMesh->GetSocketLocation(CurrentProfile.SocketNames.Last());
            WeaponLength = (End - Start).Size();
        }
    }
    
    float AngularSpeed = AngleDelta * WeaponLength;
    
    return LinearSpeed + AngularSpeed;
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

bool UWeaponCollisionComponent::ValidateHit(AActor* HitActor, const FHitResult& HitResult)
{
    if (!HitActor || !OwnerWeapon)
        return false;
    
    // 자기 자신과 소유자 제외
    AActor* WeaponOwner = OwnerWeapon->GetOwner();
    if (HitActor == OwnerWeapon || HitActor == WeaponOwner)
        return false;
    
    // 같은 팀 체크 (필요시 구현)
    
    // 거리 검증
    float Distance = (HitResult.Location - OwnerWeapon->GetActorLocation()).Size();
    if (Distance > MaxHitDistance)
    {
        DEBUG_LOG(TEXT("Hit rejected: Too far (%.2f > %.2f)"), Distance, MaxHitDistance);
        return false;
    }
    
    // 중복 히트 체크
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (FHitValidationData* ValidationData = HitValidationMap.Find(HitActor))
    {
        if (CurrentTime - ValidationData->LastHitTime < HitCooldownTime)
        {
            DEBUG_LOG(TEXT("Hit rejected: Cooldown (%.2f < %.2f)"), 
                     CurrentTime - ValidationData->LastHitTime, HitCooldownTime);
            return false;
        }
        
        // 쿨다운 지났으면 업데이트
        ValidationData->LastHitTime = CurrentTime;
        ValidationData->HitCount++;
    }
    else
    {
        // 새로운 액터 추가
        FHitValidationData NewData;
        NewData.HitActor = HitActor;
        NewData.LastHitTime = CurrentTime;
        NewData.HitCount = 1;
        HitValidationMap.Add(HitActor, NewData);
    }
    
    return true;
}

float UWeaponCollisionComponent::CalculateAdaptiveTraceRate() const
{
    float SwingSpeed = CalculateSwingSpeed();
    
    // 속도에 따라 트레이스 레이트 조정
    float AdaptiveRate = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, SwingSpeedThreshold),
        FVector2D(BaseTraceRate, MaxTraceRate),
        SwingSpeed
    );
    
    return AdaptiveRate;
}

void UWeaponCollisionComponent::DrawDebugSweptVolume(const FVector& StartPrev, const FVector& StartCurr,
                                                    const FVector& EndPrev, const FVector& EndCurr,
                                                    float Radius, const FColor& Color)
{
    // 사다리꼴 와이어프레임 그리기
    DrawDebugLine(GetWorld(), StartPrev, EndPrev, Color, false, DebugTraceDuration);
    DrawDebugLine(GetWorld(), StartCurr, EndCurr, Color, false, DebugTraceDuration);
    DrawDebugLine(GetWorld(), StartPrev, StartCurr, Color, false, DebugTraceDuration);
    DrawDebugLine(GetWorld(), EndPrev, EndCurr, Color, false, DebugTraceDuration);
    
    // 모서리에 구체 그리기
    DrawDebugSphere(GetWorld(), StartPrev, Radius, 8, Color, false, DebugTraceDuration);
    DrawDebugSphere(GetWorld(), StartCurr, Radius, 8, Color, false, DebugTraceDuration);
    DrawDebugSphere(GetWorld(), EndPrev, Radius, 8, Color, false, DebugTraceDuration);
    DrawDebugSphere(GetWorld(), EndCurr, Radius, 8, Color, false, DebugTraceDuration);
}

ECollisionChannel UWeaponCollisionComponent::GetTraceChannel() const
{
    // 프로젝트 설정에서 커스텀 채널 사용
    // DefaultEngine.ini에서 설정 필요
    return ECC_GameTraceChannel1;  // WeaponTrace 채널
}

FCollisionQueryParams UWeaponCollisionComponent::GetCollisionQueryParams() const
{
    FCollisionQueryParams Params(TEXT("WeaponTrace"), false);
    
    if (OwnerWeapon)
    {
        Params.AddIgnoredActor(OwnerWeapon);
        
        if (AActor* WeaponOwner = OwnerWeapon->GetOwner())
        {
            Params.AddIgnoredActor(WeaponOwner);
        }
    }
    
    return Params;
}