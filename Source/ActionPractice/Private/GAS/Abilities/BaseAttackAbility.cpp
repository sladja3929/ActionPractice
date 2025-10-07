#include "GAS/Abilities/BaseAttackAbility.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Items/WeaponDataAsset.h"
#include "AbilitySystemComponent.h"
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

void UBaseAttackAbility::ActivateInitSettings()
{
    Super::ActivateInitSettings();
    
    WeaponAttackData = FWeaponAbilityStatics::GetAttackDataFromAbility(this);
    if (!WeaponAttackData)
    {
        DEBUG_LOG(TEXT("Cannot Load Base Attack Data"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    ComboCounter = 0;
}

bool UBaseAttackAbility::ConsumeStamina()
{
    SetStaminaCost(WeaponAttackData->ComboAttackData[ComboCounter].StaminaCost);
    
    return Super::ConsumeStamina();
}

void UBaseAttackAbility::PlayAction()
{
    SetHitDetectionConfig();

    Super::PlayAction();
}

UAnimMontage* UBaseAttackAbility::SetMontageToPlayTask()
{
    if (ComboCounter < 0) ComboCounter = 0;
    return WeaponAttackData->AttackMontages[ComboCounter].Get();
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
