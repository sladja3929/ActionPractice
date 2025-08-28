#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BaseAttackAbility.h"
#include "Engine/Engine.h"
#include "Tasks/AbilityTask_PlayMontageWithEvents.h"
#include "ChargeAttackAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API UChargeAttackAbility : public UBaseAttackAbility
{
	GENERATED_BODY()

public:
#pragma region "Public Functions" //==================================================
	
	UChargeAttackAbility();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

#pragma endregion

protected:
#pragma region "Protected Vriables" //================================================

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

	UPROPERTY()
	bool bMaxCharged = false;

	UPROPERTY()
	bool bIsCharging = false;

	UPROPERTY()
	bool bNoCharge = false;
	
#pragma endregion

#pragma region "Protected Functions" //================================================

	virtual void ExecuteMontageTask(UAnimMontage* MontageToPlay) override;
	
	void PlayNextChargeMontage();
	
	// ===== Task Event Handler Functions =====
	virtual void OnTaskMontageCompleted() override;
	
	UFUNCTION()
	void OnNotifyEnableBufferInput();

	virtual void OnNotifyActionRecoveryEnd() override;

	UFUNCTION()
	void OnNotifyResetCombo();

	UFUNCTION()
	void OnNotifyChargeStart();
	
#pragma endregion

private:
#pragma region "Private Variables"

#pragma endregion

#pragma region "Private Functions"

#pragma endregion
};