#include "GAS/Abilities/AttackAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Items/Weapon.h"
#include "GAS/Abilities/Tasks/AbilityTask_PlayNormalAttackMontage.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogAbilitySystemComponent, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

UAttackAbility::UAttackAbility()
{
    StaminaCost = 15.0f;
}

void UAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        DEBUG_LOG(TEXT("Cannot Commit Ability"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 무기 정보 가져오기
    AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
    if (!Character)
    {
        DEBUG_LOG(TEXT("No Character"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    AWeapon* Weapon = Character->GetRightWeapon();
    if (!Weapon)
    {
        DEBUG_LOG(TEXT("No Weapon"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    FGameplayTag MainTag = this->AbilityTags.First();
    if (!MainTag.IsValid())
    {
        DEBUG_LOG(TEXT("No Ability MainTag"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    WeaponAttackData = Weapon->GetWeaponAttackDataByTag(MainTag);
    if (WeaponAttackData->AttackMontages.IsEmpty() || !WeaponAttackData->AttackMontages[0])
    {
        DEBUG_LOG(TEXT("No Montage Data - ListEmpty: %d, FirstEmpty: %d"), WeaponAttackData->AttackMontages.IsEmpty(), !WeaponAttackData->AttackMontages[0]);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
    
    // 몽타주 배열과 콤보 데이터 배열 크기 일치 검증
    if (WeaponAttackData->AttackMontages.Num() != WeaponAttackData->ComboAttackData.Num())
    {
        UE_LOG(LogAbilitySystemComponent, Error, TEXT("AttackMontages and ComboAttackData array size mismatch!"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 스태미나 소모
    if (!ConsumeStamina())
    {
        DEBUG_LOG(TEXT("No Stamina"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    // 커스텀 태스크 생성 - 몽타주 배열 전달
    NormalAttackTask = UAbilityTask_PlayNormalAttackMontage::CreatePlayNormalAttackMontageProxy(
        this,
        NAME_None,
        WeaponAttackData->AttackMontages,
        1.0f,
        NAME_None,
        1.0f
    );

    if (NormalAttackTask)
    {
        // 태스크 세팅 - 몽타주 배열 크기로 설정
        NormalAttackTask->MaxComboCount = WeaponAttackData->AttackMontages.Num();
        NormalAttackTask->ComboCounter = 0;
        
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
        DEBUG_LOG(TEXT("No NormalAttackTask"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
    }
}

void UAttackAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
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
    DEBUG_LOG(TEXT("Task Completed - EndAbility"));
}

void UAttackAbility::OnTaskInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    DEBUG_LOG(TEXT("Task Interrupted - EndAbility"));
}

void UAttackAbility::OnComboPerformed()
{
    // 콤보 실행시 필요한 추가 처리
    // 예: VFX, SFX, 카메라 효과 등
}

void UAttackAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
    DEBUG_LOG(TEXT("AttackAbility Cancelled"));
    
    // 태스크가 활성화되어 있다면 외부 취소 호출
    if (NormalAttackTask && NormalAttackTask->IsActive())
    {
        DEBUG_LOG(TEXT("Cancelling NormalAttackTask"));
        NormalAttackTask->ExternalCancel();
    }
    
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    DEBUG_LOG(TEXT("EndAbility %d"), bWasCancelled);
    if (IsEndAbilityValid(Handle, ActorInfo))
    {
        if (NormalAttackTask)
        {
            NormalAttackTask->bStopMontageWhenAbilityCancelled = bWasCancelled;
        }

        Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }
}