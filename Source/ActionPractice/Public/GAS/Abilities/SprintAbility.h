#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "SprintAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API USprintAbility : public UActionPracticeGameplayAbility
{
	GENERATED_BODY()

public:
	USprintAbility();

protected:
	
	//캐릭터에서 참조
	UPROPERTY()
	float SprintSpeedMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category="Cost")
	TSubclassOf<class UGameplayEffect> StaminaDrainEffect;

	FActiveGameplayEffectHandle StaminaDrainHandle;

public:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	virtual void StartSprinting();

	UFUNCTION(BlueprintCallable, Category = "Sprint")
	virtual void StopSprinting();

	UFUNCTION(BlueprintCallable, Category = "Sprint")
	virtual void HandleSprinting();

	UFUNCTION(BlueprintPure, Category = "Sprint")
	virtual bool CanContinueSprinting() const;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool StartStaminaDrain();

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void StopStaminaDrain();

private:
	FTimerHandle SprintCheckTimer;

	float OriginalMaxWalkSpeed = 0.0f;

	UFUNCTION()
	void CheckSprintConditions();
};