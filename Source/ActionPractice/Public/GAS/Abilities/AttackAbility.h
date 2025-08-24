#pragma once

#include "CoreMinimal.h"
#include "Public/Items/WeaponData.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "Engine/Engine.h"
#include "Tasks/AbilityTask_PlayMontageWithEvents.h"
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

	UPROPERTY()
	int32 ComboCounter = 0;

	UPROPERTY()
	int32 MaxComboCount = 1;

	UPROPERTY()
	bool bCanComboSave = false;

	UPROPERTY()
	bool bComboInputSaved = false;

	UPROPERTY()
	bool bIsInCancellableRecovery = false;
	
#pragma endregion

#pragma region "Protected Functions" //================================================

	UFUNCTION()
	void ExecuteMontageTask();
	
	void PlayNextAttackCombo();

	// ===== Task Event Handler Functions =====
	UFUNCTION()
	void OnTaskMontageCompleted();

	UFUNCTION()
	void OnTaskMontageInterrupted();

	UFUNCTION()
	void OnNotifyEnableComboInput();

	UFUNCTION()
	void OnNotifyActionRecoveryEnd();

	UFUNCTION()
	void OnNotifyResetCombo();
	
#pragma endregion

private:
#pragma region "Private Variables"

	UPROPERTY()
	UAbilityTask_PlayNormalAttackMontage* NormalAttackTask;

	UPROPERTY()
	UAbilityTask_PlayMontageWithEvents* MontageTask;
	
#pragma endregion

#pragma region "Private Functions"

#pragma endregion
};