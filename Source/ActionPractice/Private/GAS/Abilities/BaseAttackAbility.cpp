#include "GAS/Abilities/BaseAttackAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/AttributeSet/ActionPracticeAttributeSet.h"
#include "Items/WeaponDataAsset.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/ActionPracticeCharacter.h"
#include "Items/HitDetectionInterface.h"
#include "GAS/Abilities/WeaponAbilityStatics.h"
#include "GAS/Abilities/Tasks/AbilityTask_PlayMontageWithEvents.h"
#include "GAS/AbilitySystemComponent/ActionPracticeAbilitySystemComponent.h"
#include "GAS/AbilitySystemComponent/BaseAbilitySystemComponent.h"

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

    HitDetection = Character->GetHitDetectionInterface();
    if (!HitDetection)
    {
        DEBUG_LOG(TEXT("Character Not Has HitDetection System"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return; 
    }
    
    FGameplayTagContainer AssetTag = GetAssetTags();
    if (AssetTag.IsEmpty()) 
    {
        DEBUG_LOG(TEXT("No AssetTags"));
        return;
    }
    
    HitDetection->PrepareHitDetection(AssetTag, ComboCounter);
    OnHitDelegateHandle = HitDetection->GetOnHitDetected().AddUObject(this, &UBaseAttackAbility::OnHitDetected);

    DEBUG_LOG(TEXT("Attack Ability: Call Hit Detection Prepare"));
}

void UBaseAttackAbility::OnHitDetected(AActor* HitActor, const FHitResult& HitResult, FFinalAttackData AttackData)
{
    //Source ASC (공격자, AttackAbility 소유자)
    UActionPracticeAbilitySystemComponent* SourceASC = GetActionPracticeAbilitySystemComponentFromActorInfo();
    if (!HitActor || !SourceASC) return;
    
    //Target ASC (피격자)
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
    if (!TargetASC) return;
    
    //Source ASC에서 GE Spec 생성
    FGameplayEffectSpecHandle SpecHandle = SourceASC->CreateAttackGameplayEffectSpec(DamageInstantEffect, GetAbilityLevel(), this, AttackData);
    
    if (SpecHandle.IsValid())
    {        
        //Target에게 적용
        FActiveGameplayEffectHandle ActiveGEHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
        
        //적용에 성공했으면
        if (ActiveGEHandle.WasSuccessfullyApplied())
        {
            //추후 필요시 구현
        }
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
        if (OnHitDelegateHandle.IsValid() && HitDetection)
        {
            HitDetection->GetOnHitDetected().Remove(OnHitDelegateHandle);
            OnHitDelegateHandle.Reset();
        }
        
        if (PlayMontageWithEventsTask)
        {
            PlayMontageWithEventsTask->bStopMontageWhenAbilityCancelled = bWasCancelled;
        }

        Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }
}
