#include "GAS/Abilities/BaseAttackAbility.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Items/WeaponDataAsset.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Animation/AnimMontage.h"
#include "Characters/ActionPracticeCharacter.h"
#include "Items/HitDetectionInterface.h"
#include "GAS/Abilities/WeaponAbilityStatics.h"
#include "GAS/Abilities/Tasks/AbilityTask_PlayMontageWithEvents.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogBaseAttackAbility, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogBaseAttackAbility, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

UBaseAttackAbility::UBaseAttackAbility()
{
    StaminaCost = 15.0f;
}

void UBaseAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        DEBUG_LOG(TEXT("Cannot Commit Ability"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    WeaponAttackData = FWeaponAbilityStatics::GetAttackDataFromAbility(this);
    if (!WeaponAttackData)
    {
        DEBUG_LOG(TEXT("Cannot Load Base Attack Data"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ComboCounter = 0;
    PlayAction();
}


void UBaseAttackAbility::SetHitDetectionConfig()
{
    //캐릭터에 공격 정보 제공
    AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
    if (!Character)
    {
        DEBUG_LOG(TEXT("No Character"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    if (TScriptInterface<IHitDetectionInterface> HitDetection = Character->GetHitDetectionInterface())
    {
        DEBUG_LOG(TEXT("Attack Ability: Call Hit Detection Prepare"));
        FGameplayTagContainer AssetTag = GetAssetTags();
        if (!AssetTag.IsEmpty()) 
        {
            HitDetection->PrepareHitDetection(AssetTag, ComboCounter);
        }
    }
    
    else
    {
        DEBUG_LOG(TEXT("Character Not Has HitDetection System"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return; 
    }
}

void UBaseAttackAbility::PlayAction()
{
    ConsumeStaminaAndAddTag();
    SetHitDetectionConfig();
    RotateCharacter();

    WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, RotateTime);
    if (WaitDelayTask)
    {
        WaitDelayTask->OnFinish.AddDynamic(this, &UBaseAttackAbility::ExecuteMontageTask);
        WaitDelayTask->ReadyForActivation();
    }
}

void UBaseAttackAbility::ExecuteMontageTask()
{
    UAnimMontage* MontageToPlay = WeaponAttackData->AttackMontages[ComboCounter].Get();
    if (!MontageToPlay)
    {
        DEBUG_LOG(TEXT("No Montage to Play"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }
    
    // 커스텀 태스크 생성
    PlayMontageWithEventsTask = UAbilityTask_PlayMontageWithEvents::CreatePlayMontageWithEventsProxy(
        this,
        NAME_None,
        MontageToPlay,
        1.0f,
        NAME_None,
        1.0f
    );
    
    if (PlayMontageWithEventsTask)
    {        
        // 델리게이트 바인딩 - 부모 클래스의 함수 사용
        PlayMontageWithEventsTask->OnMontageCompleted.AddDynamic(this, &UBaseAttackAbility::OnTaskMontageCompleted);
        PlayMontageWithEventsTask->OnMontageInterrupted.AddDynamic(this, &UBaseAttackAbility::OnTaskMontageInterrupted);

        // 태스크 활성화
        PlayMontageWithEventsTask->ReadyForActivation();
    }
    
    else
    {
        DEBUG_LOG(TEXT("No Montage Task"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }
}     

void UBaseAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    DEBUG_LOG(TEXT("EndAbility %d"), bWasCancelled);
    
    if (IsEndAbilityValid(Handle, ActorInfo))
    {
        if (PlayMontageWithEventsTask)
        {
            PlayMontageWithEventsTask->bStopMontageWhenAbilityCancelled = bWasCancelled;
        }

        Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }
}
