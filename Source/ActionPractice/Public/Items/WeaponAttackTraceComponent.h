#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "WeaponEnums.h"
#include "AttackData.h"
#include "Items/HitDetectionInterface.h"
#include "GameplayAbilities/Public/GameplayEffectTypes.h"
#include "WeaponAttackTraceComponent.generated.h"

class UAbilitySystemComponent;
class AWeapon;
struct FWeaponDataAsset;
struct FAttackActionData;
struct FIndividualAttackData;

USTRUCT()
struct FHitValidationData
{
    GENERATED_BODY()
    
    UPROPERTY()
    TObjectPtr<AActor> HitActor = nullptr;
    
    UPROPERTY()
    float LastHitTime = 0.0f;
    
    UPROPERTY()
    int32 HitCount = 0;
};

USTRUCT()
struct FTraceConfig
{
    GENERATED_BODY()
    
    EAttackDamageType DamageType = EAttackDamageType::None;
    int32 SocketCount = 2;
    float TraceRadius = 10.0f;
    float DamageMultiplier = 1.0f;
    float StaminaDamage = 10.0f;
};

USTRUCT(BlueprintType)
struct FAdaptiveTraceConfig
{
    GENERATED_BODY()
    
    //트레이스 실행 주기 (초)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive Trace")
    float SecondsPerTrace = 0.16;

    //프레임 별 트레이스 보간 횟수 (추가)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive Trace")
    int32 InterpolationPerTrace = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive Trace")
    float SpeedThreshold = 0;
};

DECLARE_MULTICAST_DELEGATE_FourParams(FOnWeaponHit, AActor*, const FHitResult&, EAttackDamageType, float);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ACTIONPRACTICE_API UWeaponAttackTraceComponent : public UActorComponent, public IHitDetectionInterface
{
    GENERATED_BODY()

public:
#pragma region "Public Variables"

    FOnWeaponHit OnWeaponHit;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace Settings")
    FName SocketNamePrefix = FName(TEXT("trace_socket_"));
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    float DefaultTraceRadius = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace Settings")
    float HitCooldownTime = 0.1f; 

#pragma endregion

#pragma region "Public Functions"
    UWeaponAttackTraceComponent();
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                               FActorComponentTickFunction* ThisTickFunction) override;
    
    virtual void PrepareHitDetection(const FGameplayTagContainer& AttackTags, const int32 ComboIndex) override;
    UFUNCTION()
    virtual void HandleHitDetectionStart(const FGameplayEventData& Payload) override;
    UFUNCTION()
    virtual void HandleHitDetectionEnd(const FGameplayEventData& Payload) override;
    
    UFUNCTION(BlueprintCallable, Category = "Weapon Collision")
    void ResetHitActors();

    UFUNCTION(BlueprintCallable, Category = "Weapon Collision")
    void SetWeaponStaticMesh();
#pragma endregion

protected:
#pragma region "Protected Variables"

    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> CachedASC = nullptr;
    
    UPROPERTY()
    TObjectPtr<AWeapon> OwnerWeapon = nullptr;

    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> WeaponStaticMesh = nullptr;
    
    //Trace Config Variables
    FTraceConfig CurrentConfig;
    TArray<FName> TraceSocketNames;

    UPROPERTY()
    TArray<FVector> PreviousSocketPositions;

    UPROPERTY()
    TArray<FVector> CurrentSocketPositions;;
    
    UPROPERTY()
    TMap<AActor*, FHitValidationData> HitValidationMap;
    
    bool bIsTracing = false;
    bool bIsPrepared = false;
    
    //Adaptive Trace Sweep Variables
    float TraceAccumulator = 0.0f;
    float CurrentSecondsPerTrace = 1.0f;
    int32 CurrentInterpolationPerTrace = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Adaptive Trace")
    TArray<FAdaptiveTraceConfig> AdaptiveConfigs = {
        {0.05f, 1, 0.0f},
        {0.04f, 2, 500.0f},
        {0.033f, 2, 1500.0f},
        {0.025f, 3, 2500.0f},
        {0.020f, 3, 3000.0f},
        {0.016f, 3, 5000.0f},
        {0.016f, 4, 6000.0f},
        {0.016f, 5, 10000.0f}
    };

    //Event Delegate
    FDelegateHandle HitDetectionStartHandle;
    FDelegateHandle HitDetectionEndHandle;

#pragma endregion
    
#pragma region "Protected Functions"

    //Event (DetectionEnd가 RecoveryEnd보다 뒤에 일어나면 InputBuffer로 제대로 작동하지 않는 문제, 나중에 해결)
    void BindEventCallbacks();
    void UnbindEventCallbacks();

    //Trace Config
    void StartWeaponTrace();
    void StopWeaponTrace();
    bool LoadTraceConfigFromWeaponData(const FGameplayTagContainer& AttackTags, int32 ComboIndex);
    void GenerateSocketNames();


    //Main Trace
    void PerformTrace(float DeltaTime);
    void PerformSlashTrace();
    void PerformPierceTrace();
    void PerformStrikeTrace();
    bool UpdateSocketPositions();
    void PerformInterpolationTrace(
        const FVector& StartPrev, const FVector& StartCurr,
        const FVector& EndPrev, const FVector& EndCurr,
        float Radius, TArray<FHitResult>& OutHits);

    //Hit
    bool ValidateHit(AActor* HitActor, const FHitResult& HitResult, bool bIsMultiHit);
    void ProcessHit(AActor* HitActor, const FHitResult& HitResult);
    
    //Adaptive Trace Sweep
    float CalculateSwingSpeed() const;
    void UpdateAdaptiveTraceSettings();
    
    
#pragma endregion
private:
#pragma region "Private Variables"
    //속도 계산용 무기 끝 소켓 정보
    FVector PrevTipSocketLocation;
    FName TipSocketName;

#pragma endregion
    
#pragma region "Private Functions"
    // 콜리전 채널 가져오기
    ECollisionChannel GetTraceChannel() const;
    
    // 콜리전 쿼리 파라미터 설정
    FCollisionQueryParams GetCollisionQueryParams() const;
#pragma endregion

#pragma region "Debug And Profiling"
public:
    // 디버그 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DebugTrace")
    bool bDrawDebugTrace = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DebugTrace")
    float DebugTraceDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DebugTrace")
    FColor DebugTraceColor = FColor::Red;
    
    int32 DebugSweepTraceCounter = 0;
    
    void DrawDebugSweepTrace(const FVector& StartPrev, const FVector& StartCurr,
                             const FVector& EndPrev, const FVector& EndCurr,
                             float Radius, const FColor& Color);

    void ToggleWeaponDebugTrace();
#pragma endregion
};