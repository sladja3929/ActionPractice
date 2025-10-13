#include "GAS/ActionPracticeAbilitySystemGlobals.h"
#include "GAS/Effects/ActionPracticeGameplayEffectContext.h"

FGameplayEffectContext* UActionPracticeAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FActionPracticeGameplayEffectContext();
}