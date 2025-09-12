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

    // 노티파이 이벤트 델리게이트들
    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnEnableBufferInput;

    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnActionRecoveryEnd;

    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnResetCombo;

    UPROPERTY(BlueprintAssignable)
    FMontageWithEventsDelegate OnChargeStart;
    
    // 어빌리티가 취소되면 몽타주 정지
    UPROPERTY()
    bool bStopMontageWhenAbilityCancelled;

    UPROPERTY()
    bool bStopBroadCastMontageEvents;
    
#pragma endregion

#pragma region "Public Functions" //==========================================================

    UAbilityTask_PlayMontageWithEvents(const FObjectInitializer& ObjectInitializer);

    // 태스크 생성 함수
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "PlayMontageWithEvents",
        HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UAbilityTask_PlayMontageWithEvents* CreatePlayMontageWithEventsProxy(
        UGameplayAbility* OwningAbility,
        FName TaskInstanceName,
        UAnimMontage* MontageToPlay,
        float Rate = 1.0f,
        FName StartSection = NAME_None,
        float AnimRootMotionTranslationScale = 1.0f);

    // 태스크 활성화
    virtual void Activate() override;

    // 태스크 정리
    virtual void OnDestroy(bool AbilityEnded) override;

    // 태스크 외부 정지
    virtual void ExternalCancel() override;

    //몽타주 변경
    UFUNCTION()
    void ChangeMontageAndPlay(UAnimMontage* NewMontage);
    
#pragma endregion

protected:
#pragma region "Protected Variables" //=============================================================

    // 재생할 몽타주
    UPROPERTY()
    TObjectPtr<UAnimMontage> MontageToPlay;

    // 재생 속도
    UPROPERTY()
    float Rate;

    // 시작 섹션
    UPROPERTY()
    FName StartSectionName;

    // 루트 모션 스케일
    UPROPERTY()
    float AnimRootMotionTranslationScale;

    // 몽타주 델리게이트 핸들
    FOnMontageBlendingOutStarted BlendingOutDelegate;
    FOnMontageEnded MontageEndedDelegate;

    // 이벤트 핸들
    FDelegateHandle EnableBufferInputHandle;
    FDelegateHandle ActionRecoveryEndHandle;
    FDelegateHandle ResetComboHandle;
    FDelegateHandle ChargeStartHandle;
    
#pragma endregion
    
#pragma region "Protected Functions" //=============================================================
    
    UFUNCTION()
    void PlayMontage();

    //몽타주 종료
    UFUNCTION()
    void StopPlayingMontage();
    
    // 이벤트 핸들러
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

    // 이벤트 핸들 등록/해제
    void BindEventCallbacks();
    void UnbindEventCallbacks();

    // 몽타주 델리게이트 바인딩/해제
    void BindMontageCallbacks();
    void UnbindMontageCallbacks();
    
#pragma endregion
};