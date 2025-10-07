#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/ActionRecoveryAbility.h"
#include "Engine/Engine.h"
#include "BaseAttackAbility.generated.h"

class UAbilityTask_PlayMontageWithEvents;
class UAbilityTask_WaitGameplayEvent;

struct FAttackActionData;
UCLASS()
class ACTIONPRACTICE_API UBaseAttackAbility : public UActionRecoveryAbility
{
	GENERATED_BODY()

public:
#pragma region "Public Functions" //==================================================
	
	UBaseAttackAbility();
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
#pragma endregion

protected:
#pragma region "Protected Vriables" //================================================
	
	const FAttackActionData* WeaponAttackData = nullptr;

	UPROPERTY()
	int32 ComboCounter = 0;

	UPROPERTY()
	int32 MaxComboCount = 0;
	
#pragma endregion

#pragma region "Protected Functions" //================================================
	
	UFUNCTION()
	virtual void SetHitDetectionConfig();
	
	virtual void ActivateInitSettings() override;
	virtual bool ConsumeStamina() override;
	virtual void PlayAction() override;
	virtual UAnimMontage* SetMontageToPlayTask() override;
	
#pragma endregion

private:
#pragma region "Private Variables"

	
#pragma endregion

#pragma region "Private Functions"

#pragma endregion
};