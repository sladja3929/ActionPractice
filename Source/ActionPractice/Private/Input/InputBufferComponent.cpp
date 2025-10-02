#include "Input/InputBufferComponent.h"
#include "Characters/ActionPracticeCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "GAS/GameplayTagsSubsystem.h"
#include "EnhancedInputComponent.h"
#include "Input/InputActionDataAsset.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogInputBufferComponent, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogInputBufferComponent, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

UInputBufferComponent::UInputBufferComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInputBufferComponent::BeginPlay()
{
	AActor* CurrentOwner = GetOwner();
	OwnerCharacter = Cast<AActionPracticeCharacter>(CurrentOwner);

	if (!OwnerCharacter)
	{
		return;
	}

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (ASC)
	{
		EnableBufferInputHandle = ASC->GenericGameplayEventCallbacks
		.FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyEnableBufferInputTag())
		.AddLambda([this](const FGameplayEventData* EventData)
			{
				if (IsValid(this) && EventData)
				{
					OnEnableBufferInput(*EventData);
				}
			});

		ActionRecoveryEndHandle = ASC->GenericGameplayEventCallbacks
		.FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyActionRecoveryEndTag())
		.AddLambda([this](const FGameplayEventData* EventData)
			{
				if (IsValid(this) && EventData)
				{
					OnActionRecoveryEnd(*EventData);
				}
			});
	}
	
	Super::BeginPlay();
}

bool UInputBufferComponent::CanBufferAction(const UInputAction* InputAction, int32& OutPriority, bool& bIsHoldAction) const
{
	if (!InputAction || !OwnerCharacter)
	{
		OutPriority = -1;
		bIsHoldAction = false;
		return false;
	}

	const UInputActionDataAsset* InputActionData = OwnerCharacter->GetInputActionData();
	if (!InputActionData)
	{
		OutPriority = -1;
		bIsHoldAction = false;
		return false;
	}
	
	const FInputActionAbilityRule* InputActionRule = InputActionData->FindRuleByAction(InputAction);
	if (!InputActionRule)
	{
		OutPriority = -1;
		bIsHoldAction = false;
		return false;
	}
	
	OutPriority = InputActionRule->BufferPriority;
	bIsHoldAction = InputActionRule->bIsHoldAction;
	return InputActionRule->bCanBuffered;
}

void UInputBufferComponent::BufferNextAction(const UInputAction* InputedAction)
{
	if (!bCanBufferInput) return;

	int32 NewActionPriority;
	bool bIsHoldAction;
	if (!CanBufferAction(InputedAction, NewActionPriority, bIsHoldAction))
	{
		DEBUG_LOG(TEXT("Cannot buffer action, bCanBuffered is false - Action: %s"), *InputedAction->GetName());
		return;
	}

	if (bIsHoldAction)
	{
		BufferedHoldAction.Add(InputedAction);
		DEBUG_LOG(TEXT("Buffered hold action added - Action: %s"), *InputedAction->GetName());
	}
	
	else if (NewActionPriority >= CurrentBufferPriority)
	{
		bBufferActionReleased = false;
		BufferedAction = InputedAction;
		CurrentBufferPriority = NewActionPriority;
		DEBUG_LOG(TEXT("Buffered action updated - Action: %s"), *InputedAction->GetName());
	}
	else
	{
		DEBUG_LOG(TEXT("Buffer action ignored - Lower priority: %d vs %d"), NewActionPriority, CurrentBufferPriority);
	}
}

void UInputBufferComponent::UnBufferHoldAction(const UInputAction* InputedAction)
{
	bBufferActionReleased = true;
	BufferedHoldAction.Remove(InputedAction);
	DEBUG_LOG(TEXT("Buffered hold action removed - Action: %s"), *InputedAction->GetName());
}

void UInputBufferComponent::ActivateAbility(const UInputAction* InputAction)
{
	if (!InputAction || !OwnerCharacter) return;
	
	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!ASC) return;

	TArray<FGameplayAbilitySpec*> TryActivateSpecs = OwnerCharacter->FindAbilitySpecsWithInputAction(InputAction);
	if (TryActivateSpecs.IsEmpty()) return;

	for (auto& Spec : TryActivateSpecs)
	{
		if (Spec->IsActive()) //어빌리티가 실행중이면 (콤보 공격)
		{
			DEBUG_LOG(TEXT("Play Buffer - Play Buffer Event: %s"), *GetNameSafe(Spec->Ability->GetClass()));
			Spec->InputPressed = true;

			//현재 Spec인 어빌리티만 OnPlayBuffer가 활성화되도록 자기 자신을 EventData로 넘김
			UGameplayAbility* Instance = Spec->GetPrimaryInstance();
			if (!Instance) Instance = Spec->Ability;
			
			FGameplayEventData EventData;
			EventData.OptionalObject = Instance;			
			ASC->HandleGameplayEvent(UGameplayTagsSubsystem::GetEventActionPlayBufferTag(), &EventData);
		}

		else
		{
			DEBUG_LOG(TEXT("Play Buffer - Activate Ability: %s"), *GetNameSafe(Spec->Ability->GetClass()));
			Spec->InputPressed = true;
			ASC->TryActivateAbility(Spec->Handle);
		}
	}
}

void UInputBufferComponent::ActivateBufferAction()
{
	bCanBufferInput = false;

	//일반버퍼와 홀드버퍼가 같이 있다면 버퍼만 실행, 홀드버퍼는 일반버퍼 액션 실행 후 일반버퍼가 비어있다면 실행
	if (BufferedAction)
	{
		ActivateAbility(BufferedAction);
		
		BufferedAction = nullptr;
		CurrentBufferPriority = -1;
	}
	
	else if (BufferedHoldAction.Num() > 0)
	{
		for (const UInputAction* InputAction : BufferedHoldAction)
		{
			ActivateAbility(InputAction);
		}
		
		BufferedHoldAction.Empty();
	}	
}

bool UInputBufferComponent::IsBufferWaiting()
{
	return BufferedAction != nullptr;
}

void UInputBufferComponent::OnActionRecoveryStart(const FGameplayEventData& EventData)
{
	if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		ASC->AddLooseGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag());
	}
	DEBUG_LOG(TEXT("Action Recovery Start - Add State.Recovering"));
}

void UInputBufferComponent::OnEnableBufferInput(const FGameplayEventData& EventData)
{
	bCanBufferInput = true;
	DEBUG_LOG(TEXT("Enable Buffer Input - Can Buffer Action"));
}

void UInputBufferComponent::OnActionRecoveryEnd(const FGameplayEventData& EventData)
{
	bCanBufferInput = false;
	
	if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		//모든 StateRecovering 태그 제거 (스택된 태그 모두 제거)
		while (ASC->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag()))
		{
			ASC->RemoveLooseGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag());
		}
		DEBUG_LOG(TEXT("Action Recovery End - Remove State.Recovering"));
	}
	
	if (BufferedAction || BufferedHoldAction.Num() > 0) //저장한 행동이 있을 경우
	{
		DEBUG_LOG(TEXT("Action Recovery End - Play Buffer"));
		ActivateBufferAction();
	}

	else DEBUG_LOG(TEXT("Action Recovery End - No Buffered Action"));
}

void UInputBufferComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (ASC)
	{
		if (EnableBufferInputHandle.IsValid())
		{
			ASC->GenericGameplayEventCallbacks.FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyEnableBufferInputTag())
				.Remove(EnableBufferInputHandle);
		}

		if (ActionRecoveryEndHandle.IsValid())
		{
			ASC->GenericGameplayEventCallbacks.FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyActionRecoveryEndTag())
				.Remove(ActionRecoveryEndHandle);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}