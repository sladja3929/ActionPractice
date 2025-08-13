#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "JumpAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API UJumpAbility : public UActionPracticeGameplayAbility
{
	GENERATED_BODY()

public:
	UJumpAbility();

protected:
	// 점프 강도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump")
	float JumpZVelocity = 600.0f;

	// 더블 점프 허용 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump")
	bool bAllowDoubleJump = false;

	// 더블 점프 강도 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump")
	float DoubleJumpMultiplier = 0.8f;

	// 최대 점프 횟수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump")
	int32 MaxJumpCount = 1;

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

	// 더블 점프 실행
	UFUNCTION(BlueprintCallable, Category = "Jump")
	virtual void PerformDoubleJump();

	// 점프 가능 여부 확인
	UFUNCTION(BlueprintPure, Category = "Jump")
	virtual bool CanJump() const;

	// 더블 점프 가능 여부 확인
	UFUNCTION(BlueprintPure, Category = "Jump")
	virtual bool CanDoubleJump() const;

	// 착지 감지
	UFUNCTION(BlueprintCallable, Category = "Jump")
	virtual void OnLanded();

private:
	// 현재 점프 횟수 (const 함수에서도 접근 가능하도록)
	UPROPERTY()
	mutable int32 CurrentJumpCount = 0;

	// 점프 시작 시간
	float JumpStartTime = 0.0f;

	// 착지 확인 타이머
	FTimerHandle LandingCheckTimer;

	// 착지 확인 함수
	UFUNCTION()
	void CheckForLanding();
};