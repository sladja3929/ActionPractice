#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/ActionRecoveryAbility.h"
#include "RollAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;

UCLASS()
class ACTIONPRACTICE_API URollAbility : public UActionRecoveryAbility
{
	GENERATED_BODY()

public:
#pragma region "Public Functions"
	URollAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
#pragma endregion

protected:
#pragma region "Protected Variables"
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	class UAnimMontage* RollMontage;

	//무적 상태 Gameplay Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	TSubclassOf<class UGameplayEffect> InvincibilityEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float InvincibilityDuration = 0.5f;

	//JustRolled 태그 부여용 Effect
	UPROPERTY(EditDefaultsOnly, Category="Roll")
	TSubclassOf<UGameplayEffect> JustRolledWindowEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float JustRolledWindowDuration = 0.1f;
#pragma endregion

#pragma region "Protected Functions"

	virtual void ExecuteMontageTask() override;
	
	UFUNCTION()
	virtual void OnNotifyInvincibleStart(FGameplayEventData Payload);

	// 무적 상태 적용
	UFUNCTION(BlueprintCallable, Category = "Roll")
	virtual void ApplyInvincibilityEffect();
#pragma endregion

private:
#pragma region "Private Variables"
	// 무적 이벤트 대기 태스크
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitInvincibleStartEventTask;

	// 무적 이펙트 핸들
	FActiveGameplayEffectHandle InvincibilityEffectHandle;
#pragma endregion

#pragma region "Private Functions"
#pragma endregion
};