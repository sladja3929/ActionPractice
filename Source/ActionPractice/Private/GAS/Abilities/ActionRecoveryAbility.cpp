#include "GAS/Abilities/ActionRecoveryAbility.h"
#include "Input/InputBufferComponent.h"
#include "Characters/ActionPracticeCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/GameplayTagsSubsystem.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogActionRecoveryAbility, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogActionRecoveryAbility, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

UActionRecoveryAbility::UActionRecoveryAbility()
{
	
}

void UActionRecoveryAbility::ConsumeStaminaAndAddTag()
{
	// 스태미나 소모
	if (!ApplyStaminaCost())
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
}

void UActionRecoveryAbility::RotateCharacter()
{
	// 캐릭터 회전
	if (AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo())
	{
		Character->RotateCharacterToInputDirection(RotateTime);
	}
}

void UActionRecoveryAbility::PlayAction()
{
	ConsumeStaminaAndAddTag();
	RotateCharacter();

	WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, RotateTime);
	if (WaitDelayTask)
	{
		WaitDelayTask->OnFinish.AddDynamic(this, &UActionRecoveryAbility::ExecuteMontageTask);
		WaitDelayTask->ReadyForActivation();
	}
}

void UActionRecoveryAbility::BindAndReadyPlayBufferEvent()
{
	//이벤트 태스크 실행
	WaitPlayBufferEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, UGameplayTagsSubsystem::GetEventActionPlayBufferTag(), nullptr, false, true);
	WaitPlayBufferEventTask->EventReceived.AddDynamic(this, &UActionRecoveryAbility::OnEventPlayBuffer);
	WaitPlayBufferEventTask->ReadyForActivation();
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