#pragma once

#include "CoreMinimal.h"
#include "Public/Items/WeaponData.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "Engine/Engine.h"
#include "Tasks/AbilityTask_PlayNormalAttackMontage.h"
#include "AttackAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API UAttackAbility : public UActionPracticeGameplayAbility
{
	GENERATED_BODY()

public:
#pragma region "Public Functions" //==================================================
	
	UAttackAbility();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

#pragma endregion

protected:
#pragma region "Protected Vriables" //================================================
	
	const FAttackActionData* WeaponAttackData;

	
#pragma endregion

#pragma region "Protected Functions" //================================================

	// ===== Task Evenet Handler Functions =====
	
	UFUNCTION()
	void OnTaskCompleted();

	UFUNCTION()
	void OnTaskInterrupted();
	
	UFUNCTION()
	void OnComboPerformed();
	
#pragma endregion

private:
#pragma region "Private Variables"

	UPROPERTY()
	UAbilityTask_PlayNormalAttackMontage* NormalAttackTask;
	
#pragma endregion

#pragma region "Private Functions"

#pragma endregion
};