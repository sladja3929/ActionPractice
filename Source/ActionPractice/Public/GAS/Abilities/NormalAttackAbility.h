#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BaseAttackAbility.h"
#include "Engine/Engine.h"
#include "NormalAttackAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API UNormalAttackAbility : public UBaseAttackAbility
{
	GENERATED_BODY()

public:
#pragma region "Public Functions" //==================================================
	
	UNormalAttackAbility();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

#pragma endregion

protected:
#pragma region "Protected Vriables" //================================================

	UPROPERTY()
	int32 ComboCounter = 0;

	UPROPERTY()
	int32 MaxComboCount = 1;
	
#pragma endregion

#pragma region "Protected Functions" //================================================

	virtual void ExecuteMontageTask(UAnimMontage* MontageToPlay, bool bCreateTask);
	
	void PlayNextAttackCombo();

	// ===== Task Event Handler Functions =====
	UFUNCTION()
	void OnNotifyResetCombo();

	virtual void OnEventPlayBuffer(FGameplayEventData Payload) override;
	
#pragma endregion

private:
#pragma region "Private Variables"

#pragma endregion

#pragma region "Private Functions"

#pragma endregion
};