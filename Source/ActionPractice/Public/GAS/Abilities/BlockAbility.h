#pragma once

#include "CoreMinimal.h"
#include "MontageAbilityInterface.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "BlockAbility.generated.h"

struct FBlockActionData;
class UAbilityTask_PlayMontageWithEvents;

UCLASS()
class ACTIONPRACTICE_API UBlockAbility : public UActionPracticeGameplayAbility, public IMontageAbilityInterface
{
	GENERATED_BODY()
	
public:
#pragma region "Public Functions"
	UBlockAbility();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

#pragma endregion

protected:
#pragma region "Protected Variables"

	const FBlockActionData* WeaponBlockData;
	
	UPROPERTY()
	UAbilityTask_PlayMontageWithEvents* PlayMontageWithEventsTask;
	
	UPROPERTY()
	UAnimMontage* MontageToPlay = nullptr;
	
	UPROPERTY()
	float BlockAngle = 120.0f; // 정면 120도

	UPROPERTY()
	float ParryWindow = 0.5f;

	UPROPERTY()
	float DamageReductionMultiplier = 0.8f; // 20% 데미지만 받음

	UPROPERTY()
	float StaminaDamageReduction = 0.5f; // 50% 스태미나 데미지만 받음
	
	UPROPERTY()
	float BlockStartTime = 0.0f;

	UPROPERTY()
	bool bIsBlocking = false;

	//ExecuteMontageTask 파라미터
	bool bCreateTask = false;
	
#pragma endregion

#pragma region "Protected Functions"

	UFUNCTION()
	virtual void PlayAction() override;
	
	UFUNCTION()
	virtual void ExecuteMontageTask() override;

	//Idle일 때는 호출되지 않음, 오로지 Reaction일때만
	UFUNCTION()
	virtual void OnTaskMontageCompleted() override;
	
	UFUNCTION()
	virtual void OnTaskMontageInterrupted() override;
	
#pragma endregion

private:
#pragma region "Private Variables"
#pragma endregion

#pragma region "Private Functions"
#pragma endregion
};