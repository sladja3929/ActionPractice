#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BaseAttackAbility.h"
#include "Engine/Engine.h"
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
	bool bMaxCharged = false;

	UPROPERTY()
	bool bIsCharging = false;

	UPROPERTY()
	bool bNoCharge = false;

	//PlayAction, ExecuteMontageTask 파라미터
	UPROPERTY()
	bool bCreateTask = false;

	UPROPERTY()
	bool bIsAttackMontage = false;
	
#pragma endregion

#pragma region "Protected Functions" //================================================

	virtual void SetStaminaCost(float InStaminaCost) override;
	virtual void RotateCharacter() override;
	virtual void ExecuteMontageTask() override;
	virtual void PlayAction() override;

	UFUNCTION()
	void PlayNextCharge();
	
	// ===== Task Event Handler Functions =====
	virtual void OnTaskMontageCompleted() override;

	UFUNCTION()
	void OnNotifyResetCombo();

	UFUNCTION()
	void OnNotifyChargeStart();

	virtual void OnEventPlayBuffer(FGameplayEventData Payload) override;
	
#pragma endregion

private:
#pragma region "Private Variables"

#pragma endregion

#pragma region "Private Functions"

#pragma endregion
};