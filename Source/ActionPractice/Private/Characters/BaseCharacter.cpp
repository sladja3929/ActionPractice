#include "Characters/BaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilities/Public/Abilities/GameplayAbility.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogBaseCharacter, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogBaseCharacter, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilitySystem();
}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABaseCharacter::InitializeAbilitySystem()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		GrantStartupAbilities();

		ApplyStartupEffects();
	}
}

void ABaseCharacter::GiveAbility(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (AbilitySystemComponent && AbilityClass)
	{
		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}

void ABaseCharacter::GrantStartupAbilities()
{
	for (const auto& StartAbility : StartAbilities)
	{
		GiveAbility(StartAbility);
	}
}

void ABaseCharacter::ApplyStartupEffects()
{
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (const auto& StartEffect : StartEffects)
	{
		if (StartEffect)
		{
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(StartEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}