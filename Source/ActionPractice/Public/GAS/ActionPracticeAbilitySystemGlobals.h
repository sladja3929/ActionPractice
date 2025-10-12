#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "ActionPracticeAbilitySystemGlobals.generated.h"

/**
 * Custom AbilitySystemGlobals: GAS 관리 싱글톤, APGEContext 할당을 위해 오버라이드
 */
UCLASS()
class ACTIONPRACTICE_API UActionPracticeAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};