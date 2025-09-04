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

	// 어빌리티 활성화
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 어빌리티 종료
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
#pragma endregion

protected:
#pragma region "Protected Variables"
	// 구르기 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	class UAnimMontage* RollMontage;

	// 무적 상태 Gameplay Effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	TSubclassOf<class UGameplayEffect> InvincibilityEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float InvincibilityDuration;
#pragma endregion

#pragma region "Protected Functions"

	virtual void ExecuteMontageTask() override;
	
	// 애님 노티파이 이벤트 수신 시 호출
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