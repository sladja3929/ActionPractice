#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilityTask_PlayMontageWithEvents.generated.h"

class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMontageWithEventsDelegate);

UCLASS()
class ACTIONPRACTICE_API UAbilityTask_PlayMontageWithEvents : public UAbilityTask
{
    GENERATED_BODY()

public:
#pragma region "Public Variables" //=======================================================
    
    // 델리게이트들
    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnMontageCompleted;

    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnMontageBlendOut;

    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnMontageInterrupted;

    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnTaskCancelled;

    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnEnableBufferInput;

    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnActionRecoveryEnd;

    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnResetCombo;

    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnChargeStart;
    
    UPROPERTY()
    bool bStopMontageWhenAbilityCancelled = false;

    UPROPERTY()
    bool bStopBroadCastMontageEvents = false;
    
#pragma endregion

#pragma region "Public Functions" //==========================================================

    UAbilityTask_PlayMontageWithEvents(const FObjectInitializer& ObjectInitializer);

    //태스크 생성 함수
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "PlayMontageWithEvents",
        HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UAbilityTask_PlayMontageWithEvents* CreatePlayMontageWithEventsProxy(
        UGameplayAbility* OwningAbility,
        FName TaskInstanceName,
        UAnimMontage* MontageToPlay,
        float Rate = 1.0f,
        FName StartSection = NAME_None,
        float AnimRootMotionTranslationScale = 1.0f);

    virtual void Activate() override;

    virtual void OnDestroy(bool AbilityEnded) override;

    virtual void ExternalCancel() override;

    UFUNCTION()
    void ChangeMontageAndPlay(UAnimMontage* NewMontage);
    
#pragma endregion

protected:
#pragma region "Protected Variables" //=============================================================

    UPROPERTY()
    TObjectPtr<UAnimMontage> MontageToPlay;

    UPROPERTY()
    float Rate = 0.0f;

    UPROPERTY()
    FName StartSectionName;

    UPROPERTY()
    float AnimRootMotionTranslationScale = 0.0f;

    FOnMontageBlendingOutStarted BlendingOutDelegate = nullptr;
    FOnMontageEnded MontageEndedDelegate = nullptr;;

    FDelegateHandle EnableBufferInputHandle;
    FDelegateHandle ActionRecoveryEndHandle;
    FDelegateHandle ResetComboHandle;
    FDelegateHandle ChargeStartHandle;
    
#pragma endregion
    
#pragma region "Protected Functions" //=============================================================
    
    UFUNCTION()
    void PlayMontage();

    UFUNCTION()
    void StopPlayingMontage();
    
    //이벤트 핸들러
    UFUNCTION()
    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    
    UFUNCTION()
    void HandleEnableBufferInputEvent(const FGameplayEventData& Payload);

    UFUNCTION()
    void HandleActionRecoveryEndEvent(const FGameplayEventData& Payload);

    UFUNCTION()
    void HandleResetComboEvent(const FGameplayEventData& Payload);

    UFUNCTION()
    void HandleChargeStartEvent(const FGameplayEventData& Payload);

    //이벤트 핸들 등록/해제
    void BindEventCallbacks();
    void UnbindEventCallbacks();

    //몽타주 델리게이트 바인딩/해제
    void BindMontageCallbacks();
    void UnbindMontageCallbacks();
    
#pragma endregion
};