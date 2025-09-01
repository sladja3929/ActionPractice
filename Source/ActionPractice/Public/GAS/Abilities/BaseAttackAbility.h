#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "Engine/Engine.h"
#include "BaseAttackAbility.generated.h"

class UAbilityTask_PlayMontageWithEvents;
class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_WaitDelay;

struct FAttackActionData;
UCLASS()
class ACTIONPRACTICE_API UBaseAttackAbility : public UActionPracticeGameplayAbility
{
	GENERATED_BODY()

public:
#pragma region "Public Functions" //==================================================
	
	UBaseAttackAbility();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
#pragma endregion

protected:
#pragma region "Protected Vriables" //================================================
	
	const FAttackActionData* WeaponAttackData;

	UPROPERTY()
	UAbilityTask_PlayMontageWithEvents* PlayMontageWithEventsTask;

	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitPlayBufferEventTask;

	UPROPERTY()
	UAbilityTask_WaitDelay* WaitDelayTask;

	UPROPERTY()
	UAnimMontage* MontageToPlay = nullptr;
	
#pragma endregion

#pragma region "Protected Functions" //================================================
	
	UFUNCTION()
	virtual void ExecuteMontageTask();

	UFUNCTION()
	virtual void PlayMontage();

	// ===== Task Event Handler Functions =====
	UFUNCTION()
	virtual void OnTaskMontageCompleted();

	UFUNCTION()
	virtual void OnTaskMontageInterrupted();

	UFUNCTION()
	virtual void OnEventPlayBuffer(FGameplayEventData Payload) {}
	
	// Weapon 레퍼런스
	UFUNCTION()
	class AWeapon* GetWeaponClassFromActorInfo() const;

	// WeaponData
	UFUNCTION()
	bool SetWeaponAttackDataFromActorInfo();
	
#pragma endregion

private:
#pragma region "Private Variables"

	
#pragma endregion

#pragma region "Private Functions"

#pragma endregion
};