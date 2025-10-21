#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Player/BaseAttackAbility.h"
#include "Engine/Engine.h"
#include "NormalAttackAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API UNormalAttackAbility : public UBaseAttackAbility
{
	GENERATED_BODY()

public:
#pragma region "Public Functions" //==================================================

	UNormalAttackAbility();
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

#pragma endregion

protected:
#pragma region "Protected Vriables" //================================================

	//PlayAction, ExecuteMontageTask 파라미터
	UPROPERTY()
	bool bCreateTask = false;

	//사용되는 태그들
	FGameplayTag EventNotifyResetComboTag;
	
#pragma endregion

#pragma region "Protected Functions" //================================================

	virtual void ActivateInitSettings() override;
	virtual void ExecuteMontageTask() override;
	virtual void BindEventsAndReadyMontageTask() override;
	
	UFUNCTION()
	void PlayNextAttack();
	
	virtual void OnTaskNotifyEventsReceived(FGameplayEventData Payload) override;
	
	UFUNCTION()
	void OnNotifyResetCombo(FGameplayEventData Payload);
	
	virtual void OnEventInputByBuffer(FGameplayEventData Payload) override;
	
#pragma endregion

private:
#pragma region "Private Variables"

#pragma endregion

#pragma region "Private Functions"

#pragma endregion
};