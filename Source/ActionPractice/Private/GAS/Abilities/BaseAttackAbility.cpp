#include "GAS/Abilities/BaseAttackAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Items/Weapon.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"
#include "GAS/GameplayTagsSubsystem.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogAbilitySystemComponent, Warning, Format, ##__VA_ARGS__)
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

    if (!SetWeaponAttackDataFromActorInfo())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    ExecuteMontageTask(WeaponAttackData->AttackMontages[0].Get());
}

void UBaseAttackAbility::ExecuteMontageTask(UAnimMontage* MontageToPlay)
{
    // 스태미나 소모
    if (!ConsumeStamina())
    {
        DEBUG_LOG(TEXT("No Stamina"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
    {
        AbilitySystemComponent->AddLooseGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag());
    }
    
    // 커스텀 태스크 생성
    MontageTask = UAbilityTask_PlayMontageWithEvents::CreatePlayMontageWithEventsProxy(
        this,
        NAME_None,
        MontageToPlay,
        1.0f,
        NAME_None,
        1.0f
    );
    
    if (MontageTask)
    {        
        // 델리게이트 바인딩 - 사용하지 않는 델리게이트도 있음
        MontageTask->OnMontageCompleted.AddDynamic(this, &UBaseAttackAbility::OnTaskMontageCompleted);
        MontageTask->OnMontageInterrupted.AddDynamic(this, &UBaseAttackAbility::OnTaskMontageInterrupted);
        MontageTask->OnActionRecoveryEnd.AddDynamic(this, &UBaseAttackAbility::OnNotifyActionRecoveryEnd);

        // 태스크 활성화
        MontageTask->ReadyForActivation();
    }
    
    else
    {
        DEBUG_LOG(TEXT("No Montage Task"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }
}

void UBaseAttackAbility::OnTaskMontageCompleted()
{
    DEBUG_LOG(TEXT("Task Completed - EndAbility"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UBaseAttackAbility::OnTaskMontageInterrupted()
{
    DEBUG_LOG(TEXT("Task Interrupted - EndAbility"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UBaseAttackAbility::OnNotifyActionRecoveryEnd()
{
    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
    {
        // 모든 StateRecovering 태그 제거 (스택된 태그 모두 제거)
        while (AbilitySystemComponent->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag()))
        {
            AbilitySystemComponent->RemoveLooseGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag());
        }
    }
}

bool UBaseAttackAbility::SetWeaponAttackDataFromActorInfo()
{
    AWeapon* Weapon = GetWeaponClassFromActorInfo();
    if (!Weapon)
    {
        DEBUG_LOG(TEXT("No Weapon"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return false;
    }

    FGameplayTag MainTag = this->AbilityTags.First();
    if (!MainTag.IsValid())
    {
        DEBUG_LOG(TEXT("No Ability MainTag"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return false;
    }

    WeaponAttackData = Weapon->GetWeaponAttackDataByTag(MainTag);
    if (WeaponAttackData->AttackMontages.IsEmpty() || !WeaponAttackData->AttackMontages[0])
    {
        DEBUG_LOG(TEXT("No Montage Data - ListEmpty: %d, FirstEmpty: %d"), WeaponAttackData->AttackMontages.IsEmpty(), !WeaponAttackData->AttackMontages[0]);
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return false;
    }
    
    // 몽타주 배열과 콤보 데이터 배열 크기 일치 검증
    if (WeaponAttackData->AttackMontages.Num() != WeaponAttackData->ComboAttackData.Num())
    {
        DEBUG_LOG(TEXT("AttackMontages and ComboAttackData array size mismatch!"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return false;
    }

    return true;
}

class AWeapon* UBaseAttackAbility::GetWeaponClassFromActorInfo() const
{
    AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
    if (Character)
    {
        return Character->GetRightWeapon();
    }
    return nullptr;
}

void UBaseAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    DEBUG_LOG(TEXT("EndAbility %d"), bWasCancelled);
    
    if (IsEndAbilityValid(Handle, ActorInfo))
    {
        if (MontageTask)
        {
            MontageTask->bStopMontageWhenAbilityCancelled = bWasCancelled;
        }

        Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }
}
