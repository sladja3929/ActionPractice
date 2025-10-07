#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilityTask_PlayMontageWithEvents.generated.h"

class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMontageDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEventsDelegate, FGameplayEventData, Payload);

UCLASS()
class ACTIONPRACTICE_API UAbilityTask_PlayMontageWithEvents : public UAbilityTask
{
    GENERATED_BODY()

public:
#pragma region "Public Variables" //=======================================================
    
    //외부에서 사용하는 몽타주 델리게이트
    UPROPERTY(BlueprintAssignable)
    FMontageDelegate OnMontageCompleted;

    UPROPERTY(BlueprintAssignable)
    FMontageDelegate OnMontageBlendOut;

    UPROPERTY(BlueprintAssignable)
    FMontageDelegate OnMontageInterrupted;

    UPROPERTY(BlueprintAssignable)
    FMontageDelegate OnTaskCancelled;

    //노티파이 델리게이트 (여러 태그 이벤트들을 하나의 델리게이트로 관리)
    UPROPERTY(BlueprintAssignable)
    FEventsDelegate OnNotifyEventsReceived;
    
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

    //어빌리티에서 호출하여 노티파이 이벤트 바인드/언바인드
    UFUNCTION()
    void BindNotifyEventCallbackWithTag(FGameplayTag EventTag);

    UFUNCTION()
    void UnbindNotifyEventCallbackWithTag(FGameplayTag EventTag);
    
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

    //몽타주의 상태에 의해 직접 실행되는 내부 델리게이트
    FOnMontageBlendingOutStarted OnBlendingOutInternal;
    FOnMontageEnded OnMontageEndedInternal;

    //각 이벤트 태그에 따른 이벤트 핸들러를 저장
    TMap<FGameplayTag, FDelegateHandle> EventHandles;

    //수신할 이벤트 태그를 모은 컨테이너
    UPROPERTY()
    FGameplayTagContainer EventTagsToReceive;
    
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
    void HandleNotifyEvents(const FGameplayEventData& Payload);

    //전체 이벤트 핸들 바인드/해제
    void BindAllEventCallbacks();
    void UnbindAllEventCallbacks();

    //몽타주 델리게이트 바인드/해제
    void BindMontageCallbacks();
    void UnbindMontageCallbacks();
    
#pragma endregion
};