#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "SprintAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API USprintAbility : public UActionPracticeGameplayAbility
{
	GENERATED_BODY()

public:
	USprintAbility();

protected:
	// 스프린트 속도 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	float SprintSpeedMultiplier = 1.5f;

	// 스프린트 중 스태미나 소모량 (초당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	float StaminaDrainPerSecond = 12.0f;

	// 스프린트 시작에 필요한 최소 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	float MinStaminaToStart = 10.0f;

	// 스프린트 중지를 위한 최소 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sprint")
	float MinStaminaToContinue = 5.0f;

public:
	// 어빌리티 활성화 가능 여부 확인
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// 어빌리티 활성화
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	// 어빌리티 종료
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 스프린트 시작
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	virtual void StartSprinting();

	// 스프린트 종료
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	virtual void StopSprinting();

	// 스프린트 중 처리
	UFUNCTION(BlueprintCallable, Category = "Sprint")
	virtual void HandleSprinting();

	// 스프린트 가능 여부 확인
	UFUNCTION(BlueprintPure, Category = "Sprint")
	virtual bool CanContinueSprinting() const;

private:
	// 스태미나 소모 타이머
	FTimerHandle StaminaDrainTimer;

	// 스프린트 상태 확인 타이머
	FTimerHandle SprintCheckTimer;

	// 원래 이동 속도
	float OriginalMaxWalkSpeed = 0.0f;

	// 스태미나 소모 함수
	UFUNCTION()
	void DrainStamina();

	// 스프린트 상태 확인 함수
	UFUNCTION()
	void CheckSprintConditions();
};