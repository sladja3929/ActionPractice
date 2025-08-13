#pragma once

#include "CoreMinimal.h"
#include "Public/Items/WeaponData.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "Engine/Engine.h"
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
	
#pragma endregion

protected:
#pragma region "Protected Vriables" //================================================
	

	// ===== GamePlayTag =====    
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Tags")
	FGameplayTag EventTag_EnableComboInput;
    
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Tags")
	FGameplayTag EventTag_AttackRecoveryEnd;
    
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Tags")
	FGameplayTag EventTag_ResetCombo;

	const FAttackActionData* AttackData;
	
	TSoftObjectPtr<UAnimMontage> AttackMontage;
	int32 ComboCounter = 0;
	int32 MaxComboCount = 0;
	
	bool bCanComboSave = false;
	bool bIsAttacking = false;
	bool bComboInputSaved = false;
	bool bIsResetCombo = false;
	
#pragma endregion

#pragma region "Protected Functions" //================================================
	UFUNCTION(BlueprintCallable, Category = "Attack")
	virtual void PerformAttack();
	virtual void TryAttack();
	void SaveComboInput();
	void PlayAttackMontage();
	virtual void CheckComboInput();
	virtual void EnableComboInput();
	virtual void AttackRecoveryEnd();
	void ResetCombo();
	
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// ===== Evenet Handler Function =====
	UFUNCTION()
	void HandleEnableComboInputEvent(FGameplayEventData Payload);

	UFUNCTION()
	void HandleAttackRecoveryEndEvent(FGameplayEventData Payload);

	UFUNCTION()
	void HandleResetComboEvent(FGameplayEventData Payload);
	
#pragma endregion

private:
#pragma region "Private Variables"

	
#pragma endregion

#pragma region "Private Functions"
	// 몽타주 종료 델리게이트 핸들
	FOnMontageEnded MontageEndedDelegate;

#pragma endregion
};