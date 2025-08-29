#include "Characters/InputBufferComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/GameplayTagsSubsystem.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

UInputBufferComponent::UInputBufferComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bCanBufferInput = false;
	BufferedAction = nullptr;
	OwnerCharacter = nullptr;
	bBufferActionReleased = false;
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

void UInputBufferComponent::BufferNextAction(const UInputAction* InputedAction)
{
	if (!bCanBufferInput) return;

	bBufferActionReleased = false;
	BufferedAction = InputedAction;
	//어빌리티에 따라 버퍼에 안넣는거 결정
}

void UInputBufferComponent::ActivateBufferAction()
{
	bCanBufferInput = false;

	if (!BufferedAction) return;
	
	const TMap<UInputAction*, TSubclassOf<UGameplayAbility>>& StartInputAbilities = OwnerCharacter->GetStartInputAbilities();
	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (StartInputAbilities.IsEmpty() || !ASC)	return;
	
	const TSubclassOf<UGameplayAbility>* AbilityClass = StartInputAbilities.Find(BufferedAction);
	if (!AbilityClass) return;

	
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(*AbilityClass);

	if (Spec)
	{
		Spec->InputPressed = true;
		
		if (Spec->IsActive()) //어빌리티가 실행중이면 (콤보 공격)
		{
			DEBUG_LOG(TEXT("Play Buffer - Play Buffer Event"));
			FGameplayEventData EventData;
			ASC->HandleGameplayEvent(UGameplayTagsSubsystem::GetEventActionPlayBufferTag(), &EventData);
		}

		else
		{
			DEBUG_LOG(TEXT("Play Buffer - Activate Ability"));
			ASC->TryActivateAbility(Spec->Handle);
		}
	}

	BufferedAction = nullptr;
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

void UInputBufferComponent::OnActionRecoveryEnd(const FGameplayEventData& EventData)
{
	bCanBufferInput = false;
	
	if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		// 모든 StateRecovering 태그 제거 (스택된 태그 모두 제거)
		while (ASC->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag()))
		{
			ASC->RemoveLooseGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag());
		}
		//DEBUG_LOG(TEXT("Action Recovery End - Remove State.Recovering"));
	}
	
	if (BufferedAction) // 3-1. 2~3 사이 저장한 행동이 있을 경우
	{
		DEBUG_LOG(TEXT("Action Recovery End - Play Buffer"));
		ActivateBufferAction();
	}
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