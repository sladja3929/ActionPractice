#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "WeaponEnums.h"
#include "Characters/HitDetectionInterface.h"
#include "GameplayAbilities/Public/GameplayEffectTypes.h"
#include "WeaponCollisionComponent.generated.h"

class UAbilitySystemComponent;
class AWeapon;
struct FWeaponDataAsset;
struct FAttackActionData;
struct FIndividualAttackData;

// 스윕 프레임 데이터
USTRUCT()
struct FSweptFrame
{
    GENERATED_BODY()
    
    TArray<FVector> PreviousSocketPositions;
    TArray<FVector> CurrentSocketPositions;
    float DeltaTime = 0.0f;
};

// 히트 검증 데이터
USTRUCT()
struct FHitValidationData
{
    GENERATED_BODY()
    
    UPROPERTY()
    AActor* HitActor = nullptr;
    
    UPROPERTY()
    float LastHitTime = 0.0f;
    
    UPROPERTY()
    int32 HitCount = 0;
};

// 공격별 트레이스 설정 (런타임 생성)
USTRUCT()
struct FRuntimeTraceConfig
{
    GENERATED_BODY()
    
    EAttackDamageType DamageType = EAttackDamageType::None;
    int32 SocketCount = 2;
    float TraceRadius = 10.0f;
    float DamageMultiplier = 1.0f;
    float StaminaCost = 10.0f;
    float StaminaDamage = 10.0f;
};

DECLARE_MULTICAST_DELEGATE_FourParams(FOnWeaponHit, AActor*, const FHitResult&, EAttackDamageType, float);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ACTIONPRACTICE_API UWeaponCollisionComponent : public UActorComponent, public IHitDetectionInterface
{
    GENERATED_BODY()

public:
#pragma region "Public Variables"
    // 히트 델리게이트
    FOnWeaponHit OnWeaponHit;
    
    // 트레이스 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    float BaseTraceRate = 60.0f;  // 초당 트레이스 횟수
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    float MaxTraceRate = 120.0f;  // 최대 트레이스 횟수
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    float SwingSpeedThreshold = 500.0f;  // 속도 임계값
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    float DefaultTraceRadius = 10.0f;  // 기본 트레이스 반경
    
    // 히트 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Settings")
    float HitCooldownTime = 0.1f;  // 같은 액터 재히트 쿨다운
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Settings")
    float MaxHitDistance = 300.0f;  // 최대 히트 거리

    // 디버그 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebugTrace = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugTraceDuration = 2.0f;
#pragma endregion

#pragma region "Public Functions"
    UWeaponCollisionComponent();
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                               FActorComponentTickFunction* ThisTickFunction) override;
    
    // IHitDetectionInterface 구현
    virtual void PrepareHitDetection(const FGameplayTag& AttackTag, const int32 ComboIndex) override;
    
    // 히트 리셋
    UFUNCTION(BlueprintCallable, Category = "Weapon Collision")
    void ResetHitActors();
    
    // 속도 계산
    float CalculateSwingSpeed() const;
#pragma endregion

protected:
#pragma region "Protected Functions"
    // GAS 이벤트 설정
    void SetupEventListeners();
    void CleanupEventListeners();
    
    // GAS 이벤트 핸들러
    UFUNCTION()
    void HandleHitDetectionStart(const FGameplayEventData& Payload);
    
    UFUNCTION()
    void HandleHitDetectionEnd(const FGameplayEventData& Payload);
    
    // 트레이스 시작/종료
    void StartWeaponTrace();
    void StopWeaponTrace();
    
    // WeaponData에서 설정 로드
    bool LoadTraceConfigFromWeaponData(const FGameplayTag& AttackTag, int32 ComboIndex);
    
    // 소켓 이름 생성 (trace_socket_1, trace_socket_2, ...)
    void GenerateSocketNames();
    
    // 트레이스 수행
    void PerformTrace(float DeltaTime);
    
    // 공격 유형별 트레이스
    void PerformSlashTrace(const FSweptFrame& Frame);
    void PerformPierceTrace(const FSweptFrame& Frame);
    void PerformStrikeTrace(const FSweptFrame& Frame);
    
    // 스윕 볼륨 생성
    bool CreateSweptVolume(const FVector& StartPrev, const FVector& StartCurr,
                          const FVector& EndPrev, const FVector& EndCurr,
                          float Radius, TArray<FHitResult>& OutHits);
    
    // 히트 검증
    bool ValidateHit(AActor* HitActor, const FHitResult& HitResult);
    
    // 히트 처리
    void ProcessHit(AActor* HitActor, const FHitResult& HitResult);
    
    // 소켓 위치 업데이트
    void UpdateSocketPositions();
    
    // 트레이스 레이트 계산
    float CalculateAdaptiveTraceRate() const;
    
    // 디버그 드로잉
    void DrawDebugSweptVolume(const FVector& StartPrev, const FVector& StartCurr,
                             const FVector& EndPrev, const FVector& EndCurr,
                             float Radius, const FColor& Color);
#pragma endregion

#pragma region "Protected Variables"
    // ASC 캐시
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> CachedASC = nullptr;
    
    // 이벤트 핸들
    FDelegateHandle HitDetectionStartHandle;
    FDelegateHandle HitDetectionEndHandle;
    
    // 소유 무기
    UPROPERTY()
    TObjectPtr<AWeapon> OwnerWeapon = nullptr;
    
    // 현재 설정
    FRuntimeTraceConfig CurrentConfig;
    FGameplayTag CurrentAttackTag;
    int32 CurrentComboIndex = 0;
    
    // 소켓 이름들
    TArray<FName> TraceSocketNames;
    
    // 스윕 프레임 데이터
    FSweptFrame SweptFrame;
    
    // 히트 검증 데이터
    UPROPERTY()
    TMap<AActor*, FHitValidationData> HitValidationMap;
    
    // 트레이스 상태
    bool bIsTracing = false;
    bool bIsPrepared = false;  // PrepareHitDetection이 호출되었는지
    
    // 트레이스 타이머
    float TraceAccumulator = 0.0f;
    float CurrentTraceInterval = 0.016f;  // 60fps 기본값
    float TotalTraceDuration = 0.0f;
    float ElapsedTraceTime = 0.0f;
#pragma endregion

private:
#pragma region "Private Functions"
    // 콜리전 채널 가져오기
    ECollisionChannel GetTraceChannel() const;
    
    // 콜리전 쿼리 파라미터 설정
    FCollisionQueryParams GetCollisionQueryParams() const;
#pragma endregion

#pragma region "Private Variables"
    // 이전 프레임 무기 위치 (속도 계산용)
    FVector PreviousWeaponLocation;
    
    // 이전 프레임 무기 회전 (각속도 계산용)
    FQuat PreviousWeaponRotation;
#pragma endregion
};