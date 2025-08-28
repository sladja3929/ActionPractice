#include "Characters/InputBufferComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

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
}

void UInputBufferComponent::BeginPlay()
{
	AActor* CurrentOwner = GetOwner();
	OwnerCharacter = Cast<AActionPracticeCharacter>(CurrentOwner);

	if (!OwnerCharacter)
	{
		return;
	}
	
	Super::BeginPlay();
}

void UInputBufferComponent::BufferNextAction(UInputAction* InputedAction)
{
	if (!bCanBufferInput) return;
	
	BufferedAction = InputedAction;
}

void UInputBufferComponent::ActivateBufferAction()
{
	bCanBufferInput = false;
	
	const TMap<UInputAction*, TSubclassOf<UGameplayAbility>>& StartInputAbilities = OwnerCharacter->GetStartInputAbilities();
	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (StartInputAbilities.IsEmpty() || !ASC)	return;
	
	const TSubclassOf<UGameplayAbility>* AbilityClass = StartInputAbilities.Find(BufferedAction);
	if (!AbilityClass) return;

	
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(*AbilityClass);

	if (Spec)
	{
		Spec->InputPressed = true;
		ASC->TryActivateAbility(Spec->Handle);
	}
}