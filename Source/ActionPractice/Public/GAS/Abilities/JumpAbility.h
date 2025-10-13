#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/ActionPracticeAbility.h"
#include "JumpAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API UJumpAbility : public UActionPracticeAbility
{
	GENERATED_BODY()

public:
	UJumpAbility();

public:
	// 어빌리티 활성화 가능 여부 확인
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// 어빌리티 활성화
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 어빌리티 종료
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 점프 실행
	UFUNCTION(BlueprintCallable, Category = "Jump")
	virtual void PerformJump();

	// 점프 가능 여부 확인
	UFUNCTION(BlueprintPure, Category = "Jump")
	virtual bool CanJump() const;

};