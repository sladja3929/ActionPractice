#include "GAS/Abilities/BaseAttackAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Items/Weapon.h"
#include "Items/WeaponData.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"
#include "Characters/InputBufferComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GAS/Abilities/Tasks/AbilityTask_PlayMontageWithEvents.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

UBaseAttackAbility::UBaseAttackAbility()
{
    StaminaCost = 15.0f;
    RotateTime = 0.1f;
    WeaponAttackData = nullptr;
    PlayMontageWithEventsTask = nullptr;
    WaitPlayBufferEventTask = nullptr;
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

    MontageToPlay = WeaponAttackData->AttackMontages[0].Get();
    
    PlayMontage();
}

void UBaseAttackAbility::ExecuteMontageTask()
{
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
        if (PlayMontageWithEventsTask)
        {
            PlayMontageWithEventsTask->bStopMontageWhenAbilityCancelled = bWasCancelled;
        }

        Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }
}
