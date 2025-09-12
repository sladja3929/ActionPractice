﻿#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameplayTagContainer.h"
#include "WeaponEnums.h"
#include "Items/HitDetectionInterface.h"
#include "GameplayAbilities/Public/GameplayEffectTypes.h"
#include "Items/WeaponEnums.h"
#include "WeaponCCDComponent.generated.h"

class UAbilitySystemComponent;
class AWeapon;
struct FWeaponDataAsset;

USTRUCT()
struct FHitRecord
{
    GENERATED_BODY()
    
    UPROPERTY()
    AActor* HitActor = nullptr;
    
    UPROPERTY()
    float HitTime = 0.0f;
};

DECLARE_MULTICAST_DELEGATE_FourParams(FOnWeaponCCDHit, AActor*, const FHitResult&, EAttackDamageType, float);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ACTIONPRACTICE_API UWeaponCCDComponent : public UCapsuleComponent, public IHitDetectionInterface
{
    GENERATED_BODY()

public:
#pragma region "Public Variables"
    FOnWeaponCCDHit OnWeaponHit;
    
    //히트 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Settings")
    float HitCooldownTime = 0.1f;
    
    //캡슐 크기 조정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Settings")
    float DefaultCapsuleRadius = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capsule Settings")
    float DefaultCapsuleHalfHeight = 50.0f;
#pragma endregion
 
#pragma region "Public Functions"
    UWeaponCCDComponent();
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                               FActorComponentTickFunction* ThisTickFunction) override;
    
    //IHitDetectionInterface 구현
    virtual void PrepareHitDetection(const FGameplayTagContainer& AttackTags, const int32 ComboIndex) override;
    UFUNCTION()
    virtual void HandleHitDetectionStart(const FGameplayEventData& Payload) override;
    UFUNCTION()
    virtual void HandleHitDetectionEnd(const FGameplayEventData& Payload) override;
    
    UFUNCTION(BlueprintCallable, Category = "Weapon Collision")
    void ResetHitActors();
#pragma endregion

protected:
#pragma region "Protected Variables"
    UPROPERTY()
    TObjectPtr<AWeapon> OwnerWeapon = nullptr;
    
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> CachedASC = nullptr;
    
    //현재 공격 정보
    int32 CurrentComboIndex = 0;
    EAttackDamageType CurrentDamageType = EAttackDamageType::None;
    float CurrentDamageMultiplier = 1.0f;
    
    //히트 기록
    UPROPERTY()
    TArray<FHitRecord> HitRecords;
    
    //이벤트 핸들
    FDelegateHandle HitDetectionStartHandle;
    FDelegateHandle HitDetectionEndHandle;
    
    //상태
    bool bIsDetecting = false;
    bool bIsPrepared = false;
    
    //디버그용 이전 위치 (CCD 궤적 표시)
    FVector PreviousCapsuleLocation;
    FQuat PreviousCapsuleRotation;
#pragma endregion

#pragma region "Protected Functions"
    //이벤트 바인딩
    void BindEventCallbacks();
    void UnbindEventCallbacks();
    
    //충돌 처리
    UFUNCTION()
    void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                               bool bFromSweep, const FHitResult& SweepResult);
    
    bool ValidateHit(AActor* HitActor);
    void ProcessHit(AActor* HitActor, const FHitResult& HitResult);
    
    //캡슐 설정
    void UpdateCapsuleSize(EAttackDamageType DamageType);
    bool LoadAttackConfig(const FGameplayTagContainer& AttackTags, int32 ComboIndex);
#pragma endregion

#pragma region "Debug And Profiling"
public:
    //디버그 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebugCapsule = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugDuration = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    FColor DebugColor = FColor::Red;

    void DrawDebugCCDTrajectory();
#pragma endregion
};