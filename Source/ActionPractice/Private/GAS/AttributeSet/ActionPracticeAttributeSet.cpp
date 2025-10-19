#include "GAS/AttributeSet/ActionPracticeAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogPlayerStatsWidget, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogPlayerStatsWidget, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

UActionPracticeAttributeSet::UActionPracticeAttributeSet()
{
	InitStrength(10.0f);
	InitDexterity(10.0f);
}

void UActionPracticeAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, Dexterity, COND_None, REPNOTIFY_Always);
}

void UActionPracticeAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetStrengthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); // 최소 1
	}
	else if (Attribute == GetDexterityAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); // 최소 1
	}
}

void UActionPracticeAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	if (Attribute == GetStrengthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); // 최소 1
	}
	else if (Attribute == GetDexterityAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); // 최소 1
	}
}

void UActionPracticeAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

// Rep Notify Functions
void UActionPracticeAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, Strength, OldStrength);
}

void UActionPracticeAttributeSet::OnRep_Dexterity(const FGameplayAttributeData& OldDexterity)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, Dexterity, OldDexterity);
}