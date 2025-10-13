#include "Input/InputBufferComponent.h"
#include "Characters/ActionPracticeCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/GameplayTagsSubsystem.h"
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

	//태그 초기화
	EventNotifyEnableBufferInputTag = UGameplayTagsSubsystem::GetEventNotifyEnableBufferInputTag();
	EventActionInputByBufferTag = UGameplayTagsSubsystem::GetEventActionInputByBufferTag();
	EventActionPlayBufferTag = UGameplayTagsSubsystem::GetEventActionPlayBufferTag();

	if (!EventNotifyEnableBufferInputTag.IsValid())
	{
		DEBUG_LOG(TEXT("EventNotifyEnableBufferInputTag is not valid"));
	}
	if (!EventActionInputByBufferTag.IsValid())
	{
		DEBUG_LOG(TEXT("EventActionInputByBufferTag is not valid"));
	}
	if (!EventActionPlayBufferTag.IsValid())
	{
		DEBUG_LOG(TEXT("EventActionPlayBufferTag is not valid"));
	}

	if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		EnableBufferInputHandle = ASC->GenericGameplayEventCallbacks
		.FindOrAdd(EventNotifyEnableBufferInputTag)
		.AddLambda([this](const FGameplayEventData* EventData)
			{
				if (IsValid(this) && EventData)
				{
					OnEnableBufferInput(*EventData);
				}
			});

		PlayBufferHandle = ASC->GenericGameplayEventCallbacks
		.FindOrAdd(EventActionPlayBufferTag)
		.AddLambda([this](const FGameplayEventData* EventData)
			{
				if (IsValid(this) && EventData)
				{
					OnPlayBuffer(*EventData);
				}
			});
	}

	else DEBUG_LOG(TEXT("No ASC"));
	
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
		//첫 실행이거나, bRetriggerInstancedAbility = true여서 재실행될 때
		if (ASC->TryActivateAbility(Spec->Handle))
		{
			DEBUG_LOG(TEXT("Play Buffer - Activate Ability: %s"), *GetNameSafe(Spec->Ability->GetClass()));
			Spec->InputPressed = true;
		}

		//어빌리티가 이미 실행중 / bRetriggerInstancedAbility = false여서 Try를 실패했을 때 (콤보 공격)
		else if (Spec->IsActive()) //CanActivateAbility 실패로 활성화하지 못했을 때를 거르기 위해 실행 중 체크
		{
			DEBUG_LOG(TEXT("Play Buffer - Play Buffer Event: %s"), *GetNameSafe(Spec->Ability->GetClass()));
			Spec->InputPressed = true;

			//현재 Spec인 어빌리티만 OnInputByBuffer가 활성화되도록 자기 자신을 EventData로 넘김
			UGameplayAbility* Instance = Spec->GetPrimaryInstance();
			if (!Instance) Instance = Spec->Ability;
			
			FGameplayEventData EventData;
			EventData.OptionalObject = Instance;
			//bool 값을 EventMagnitude를 통해 전달
			EventData.EventMagnitude = bBufferActionReleased ? 1.0f : 0.0f;
			EventData.EventTag = EventActionInputByBufferTag;
			
			ASC->HandleGameplayEvent(EventActionInputByBufferTag, &EventData);
		}

		//다 아닐때
		else DEBUG_LOG(TEXT("Play Buffer Activate Failed: %s"), *GetNameSafe(Spec->Ability->GetClass()));
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

void UInputBufferComponent::OnEnableBufferInput(const FGameplayEventData& EventData)
{
	bCanBufferInput = true;
	DEBUG_LOG(TEXT("Enable Buffer Input - Can Buffer Action"));
}

void UInputBufferComponent::OnPlayBuffer(const FGameplayEventData& EventData)
{
	bCanBufferInput = false;
	
	if (BufferedAction || BufferedHoldAction.Num() > 0) //저장한 행동이 있을 경우
	{
		DEBUG_LOG(TEXT("Play Buffer"));
		ActivateBufferAction();
	}

	else DEBUG_LOG(TEXT("Play Buffer - No Buffered Action"));
}

void UInputBufferComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		if (EnableBufferInputHandle.IsValid())
		{
			ASC->GenericGameplayEventCallbacks.FindOrAdd(EventNotifyEnableBufferInputTag)
				.Remove(EnableBufferInputHandle);
		}

		if (PlayBufferHandle.IsValid())
		{
			ASC->GenericGameplayEventCallbacks.FindOrAdd(EventActionPlayBufferTag)
				.Remove(PlayBufferHandle);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}