#include "GAS/Abilities/AttackAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Items/Weapon.h"
#include "GAS/Abilities/Tasks/AbilityTask_PlayNormalAttackMontage.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"

UAttackAbility::UAttackAbility()
{
    StaminaCost = 15.0f;
}

void UAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 무기 정보 가져오기
    AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
    if (!Character)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    AWeapon* Weapon = Character->GetRightWeapon();
    if (!Weapon)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    FGameplayTag MainTag = this->AbilityTags.First();
    if (!MainTag.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    WeaponAttackData = Weapon->GetWeaponAttackDataByTag(MainTag);
    if (!WeaponAttackData->AttackMontage)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 스태미나 소모
    if (!ConsumeStamina())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 커스텀 태스크 생성
    NormalAttackTask = UAbilityTask_PlayNormalAttackMontage::CreatePlayNormalAttackMontageProxy(
        this,
        NAME_None,
        WeaponAttackData->AttackMontage.Get(),
        1.0f,
        NAME_None,
        1.0f
    );

    if (NormalAttackTask)
    {
        // 태스크 세팅
        NormalAttackTask->MaxComboCount = WeaponAttackData->ComboAttackData.Num();;
        NormalAttackTask->ComboCounter = 0;
        NormalAttackTask->EventTag_AttackRecoveryEnd = EventTag_AttackRecoveryEnd;
        NormalAttackTask->EventTag_EnableComboInput = EventTag_EnableComboInput;
        NormalAttackTask->EventTag_ResetCombo = EventTag_ResetCombo;
        NormalAttackTask->AttackStateTag = AttackStateTag;
        
        // 델리게이트 바인딩
        NormalAttackTask->OnCompleted.AddDynamic(this, &UAttackAbility::OnTaskCompleted);
        NormalAttackTask->OnBlendOut.AddDynamic(this, &UAttackAbility::OnTaskCompleted);
        NormalAttackTask->OnInterrupted.AddDynamic(this, &UAttackAbility::OnTaskInterrupted);
        NormalAttackTask->OnCancelled.AddDynamic(this, &UAttackAbility::OnTaskInterrupted);
        NormalAttackTask->OnComboPerformed.AddDynamic(this, &UAttackAbility::OnComboPerformed);

        // 태스크 활성화
        NormalAttackTask->ReadyForActivation();
    }
    else
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
}

void UAttackAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, 
    const FGameplayAbilityActorInfo* ActorInfo, 
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    if (NormalAttackTask)
    {
        // 콤보 진행 전 스태미나 체크
        if (NormalAttackTask->bCanComboSave || NormalAttackTask->bIsInCancellableRecovery)
        {
            if (!ConsumeStamina())
            {
                return;
            }
        }

        // 태스크에 입력 처리 위임
        NormalAttackTask->CheckComboInputPreseed();
    }
}

void UAttackAbility::OnTaskCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAttackAbility::OnTaskInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UAttackAbility::OnComboPerformed()
{
    // 콤보 실행시 필요한 추가 처리
    // 예: VFX, SFX, 카메라 효과 등
}

void UAttackAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (IsEndAbilityValid(Handle, ActorInfo))
    {
        if (NormalAttackTask)
        {
            NormalAttackTask->bStopMontageWhenAbilityCancelled = bWasCancelled;
        }

        Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }
}