#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Player/ActionPracticeAbility.h"
#include "JumpAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API UJumpAbility : public UActionPracticeAbility
{
	GENERATED_BODY()

public:
	UJumpAbility();

public:
	
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	
	UFUNCTION(BlueprintCallable, Category = "Jump")
	virtual void PerformJump();

	UFUNCTION(BlueprintPure, Category = "Jump")
	virtual bool CanJump() const;

};