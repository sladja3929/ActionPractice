#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "WeaponEnums.h"
#include "Characters/HitDetectionInterface.h"
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
#pragma region "Public Functions"
    UWeaponCollisionComponent();
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    UFUNCTION()
    void HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload);
    
    // 트레이스 시작/종료 (AttackTag로 WeaponData에서 설정 자동 로드)
    UFUNCTION(BlueprintCallable, Category = "Weapon Collision")
    void StartWeaponTrace(const FGameplayTag& AttackTag, float Duration = 0.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon Collision")
    void StopWeaponTrace();
    
    // 히트 리셋 (콤보 시작 시)
    UFUNCTION(BlueprintCallable, Category = "Weapon Collision")
    void ResetHitActors();
    
    // 속도 계산
    float CalculateSwingSpeed() const;
#pragma endregion

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

protected:
#pragma region "Protected Functions"

    //트레이스 관련 함수
    bool LoadTraceConfigFromWeaponData(const FGameplayTag& AttackTag);
    void PerformTrace(float DeltaTime);
    float CalculateAdaptiveTraceRate() const;

    void PerformSlashTrace(const FSweptFrame& Frame);
    void PerformPierceTrace(const FSweptFrame& Frame);
    void PerformStrikeTrace(const FSweptFrame& Frame);
    
    //소켓 관련 함수
    void GenerateSocketNames();
    void UpdateSocketPositions();
    
    // 스윕 볼륨 생성
    bool CreateSweptVolume(const FVector& StartPrev, const FVector& StartCurr,
                          const FVector& EndPrev, const FVector& EndCurr,
                          float Radius, TArray<FHitResult>& OutHits);
    
    // 히트 검증
    bool ValidateHit(AActor* HitActor, const FHitResult& HitResult);
    void ProcessHit(AActor* HitActor, const FHitResult& HitResult);

    //Event Handler
    void RegisterGameplayEventCallbacks();
    void UnRegisterGameplayEventCallbacks();
    
    // 디버그
    void DrawDebugSweptVolume(const FVector& StartPrev, const FVector& StartCurr,
                             const FVector& EndPrev, const FVector& EndCurr,
                             float Radius, const FColor& Color);
    
#pragma endregion

#pragma region "Protected Variables"
    
    UPROPERTY()
    AWeapon* OwnerWeapon = nullptr;

    UPROPERTY()
    UAbilitySystemComponent* AbilitySystemComponent = nullptr;

    FDelegateHandle HitDetectionStartHandle;
    FDelegateHandle HitDetectionEndHandle;

    UPROPERTY()
    FGameplayTag AttackTag;
    
    UPROPERTY()
    FRuntimeTraceConfig CurrentConfig;

    UPROPERTY()
    TArray<FName> TraceSocketNames;
    
    UPROPERTY()
    FSweptFrame SweptFrame;
    
    UPROPERTY()
    TMap<AActor*, FHitValidationData> HitValidationMap;
    
    // 트레이스 상태
    bool bIsTracing = false;
    
    // 트레이스 타이머
    float TraceAccumulator = 0.0f;
    float CurrentTraceInterval = 0.016f;  // 60fps 기본값
    float TotalTraceDuration = 0.0f;
    float ElapsedTraceTime = 0.0f;

#pragma endregion

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