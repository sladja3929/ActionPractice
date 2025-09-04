#include "GAS/Abilities/ActionRecoveryAbility.h"

#include "Characters/InputBufferComponent.h"
#include "Characters/ActionPracticeCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

UActionRecoveryAbility::UActionRecoveryAbility()
{
	RotateTime = 0.1f;
	PlayMontageTask = nullptr;
	WaitDelayTask = nullptr;
}

void UActionRecoveryAbility::PlayAction()
{
	// 스태미나 소모
	if (!ConsumeStamina())
	{
		DEBUG_LOG(TEXT("No Stamina"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 태그 부착
	if (UInputBufferComponent* IBC = GetInputBufferComponentFromActorInfo())
	{
		FGameplayEventData EventData;
		IBC->OnActionRecoveryStart(EventData);
	}

	// 캐릭터 회전
	if (AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo())
	{
		Character->RotateCharacterToInputDirection(RotateTime);
	}

	WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, RotateTime);
	if (WaitDelayTask)
	{
		WaitDelayTask->OnFinish.AddDynamic(this, &UActionRecoveryAbility::ExecuteMontageTask);
		WaitDelayTask->ReadyForActivation();
	}
}

void UActionRecoveryAbility::OnTaskMontageCompleted()
{
	DEBUG_LOG(TEXT("Montage Task Completed"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UActionRecoveryAbility::OnTaskMontageInterrupted()
{
	DEBUG_LOG(TEXT("Montage Task Interrupted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}