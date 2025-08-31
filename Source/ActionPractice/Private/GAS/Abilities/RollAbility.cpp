#include "GAS/Abilities/RollAbility.h"

#include <Characters/InputBufferComponent.h>

#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/GameplayTagsSubsystem.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameplayEffect.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

URollAbility::URollAbility()
{
	StaminaCost = 20.0f;
	InvincibilityDuration = 0.5f;
	MontageTask = nullptr;
	WaitInvincibleStartEventTask = nullptr;
}

void URollAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		DEBUG_LOG(TEXT("Cannot Commit Roll Ability"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character || !RollMontage)
	{
		DEBUG_LOG(TEXT("No Character or RollMontage"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 스태미나 소모
	if (!ConsumeStamina())
	{
		DEBUG_LOG(TEXT("No Stamina for Roll"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (UInputBufferComponent* IBC = GetInputBufferComponentFromActorInfo())
	{
		FGameplayEventData EventData;
		IBC->OnActionRecoveryStart(EventData);
	}
	
	WaitInvincibleStartEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		UGameplayTagsSubsystem::GetEventNotifyInvincibleStartTag()
	);

	if (WaitInvincibleStartEventTask)
	{
		WaitInvincibleStartEventTask->EventReceived.AddDynamic(this, &URollAbility::OnNotifyInvincibleStart);
		WaitInvincibleStartEventTask->ReadyForActivation();
	}
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		RollMontage
	);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &URollAbility::OnMontageTaskCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &URollAbility::OnMontageTaskInterrupted);
		MontageTask->ReadyForActivation();
		DEBUG_LOG(TEXT("Roll Montage Task Started"));
	}
	else
	{
		DEBUG_LOG(TEXT("Failed to create Montage Task"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void URollAbility::ApplyInvincibilityEffect()
{
	if (!InvincibilityEffect)
	{
		DEBUG_LOG(TEXT("No InvincibilityEffect set in Blueprint"));
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		DEBUG_LOG(TEXT("No AbilitySystemComponent"));
		return;
	}

	// 무적 이펙트 적용
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle EffectSpec = ASC->MakeOutgoingSpec(InvincibilityEffect, 1.0f, EffectContext);

	if (EffectSpec.IsValid())
	{
		EffectSpec.Data.Get()->SetSetByCallerMagnitude(UGameplayTagsSubsystem::GetEffectInvincibilityDurationTag(), InvincibilityDuration);
		InvincibilityEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
		DEBUG_LOG(TEXT("Invincibility Effect Applied with Duration: %f"), InvincibilityDuration)
	}
	else
	{
		DEBUG_LOG(TEXT("Failed to create Invincibility Effect Spec"));
	}
}

void URollAbility::OnMontageTaskCompleted()
{
	DEBUG_LOG(TEXT("Roll Montage Task Completed"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void URollAbility::OnMontageTaskInterrupted()
{
	DEBUG_LOG(TEXT("Roll Montage Task Interrupted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void URollAbility::OnNotifyInvincibleStart(FGameplayEventData Payload)
{
	DEBUG_LOG(TEXT("Invincible Start - Event Received"));
	ApplyInvincibilityEffect();
}

void URollAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{	
	// 무적 이펙트 제거
	if (InvincibilityEffectHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(InvincibilityEffectHandle);
			InvincibilityEffectHandle = FActiveGameplayEffectHandle();
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	//버퍼에 같은 어빌리티가 있을 경우 완전한 종료 후 재시작
	if (UInputBufferComponent* IBC = GetInputBufferComponentFromActorInfo())
	{
		FGameplayEventData EventData;
		IBC->OnActionRecoveryEnd(EventData);
	}
}