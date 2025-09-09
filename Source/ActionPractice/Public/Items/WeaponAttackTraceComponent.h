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

// 공격별 트레이스 설정
USTRUCT()
struct FTraceConfig
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

    FOnWeaponHit OnWeaponHit;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    float BaseTraceRate = 60.0f;  // 초당 트레이스 횟수
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    float MaxTraceRate = 120.0f;  // 최대 트레이스 횟수
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    float SwingSpeedThreshold = 500.0f;  // 속도 임계값
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    float DefaultTraceRadius = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Settings")
    float HitCooldownTime = 0.1f; 

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
    
    UFUNCTION(BlueprintCallable, Category = "Weapon Collision")
    void ResetHitActors();

    float CalculateSwingSpeed() const;
#pragma endregion

protected:
#pragma region "Protected Functions"

    //이벤트 관련 함수
    void BindEventCallbacks();
    void UnbindEventCallbacks();
    
    UFUNCTION()
    void HandleHitDetectionStart(const FGameplayEventData& Payload);
    
    UFUNCTION()
    void HandleHitDetectionEnd(const FGameplayEventData& Payload);
    
    void StartWeaponTrace();
    void StopWeaponTrace();
    bool LoadTraceConfigFromWeaponData(const FGameplayTag& AttackTag, int32 ComboIndex);
    void GenerateSocketNames();
    
    void PerformTrace(float DeltaTime);
    void PerformSlashTrace(const FSweptFrame& Frame);
    void PerformPierceTrace(const FSweptFrame& Frame);
    void PerformStrikeTrace(const FSweptFrame& Frame);
    
    bool CreateSweptVolume(const FVector& StartPrev, const FVector& StartCurr,
                          const FVector& EndPrev, const FVector& EndCurr,
                          float Radius, TArray<FHitResult>& OutHits);
    
    bool ValidateHit(AActor* HitActor, const FHitResult& HitResult);
    
    void ProcessHit(AActor* HitActor, const FHitResult& HitResult);
    
    bool UpdateSocketPositions();
    
    // 트레이스 레이트 계산
    float CalculateAdaptiveTraceRate() const;
    
    // 디버그
    void DrawDebugSweptVolume(const FVector& StartPrev, const FVector& StartCurr,
                             const FVector& EndPrev, const FVector& EndCurr,
                             float Radius, const FColor& Color);
#pragma endregion

#pragma region "Protected Variables"

    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> CachedASC = nullptr;
    
    UPROPERTY()
    TObjectPtr<AWeapon> OwnerWeapon = nullptr;

    // 이벤트 핸들
    FDelegateHandle HitDetectionStartHandle;
    FDelegateHandle HitDetectionEndHandle;
    
    // 현재 설정
    FTraceConfig CurrentConfig;
    FGameplayTag CurrentAttackTag;
    int32 CurrentComboIndex = 0;
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