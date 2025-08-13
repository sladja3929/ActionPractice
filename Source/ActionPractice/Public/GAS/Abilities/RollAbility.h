#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "RollAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API URollAbility : public UActionPracticeGameplayAbility
{
	GENERATED_BODY()

public:
	URollAbility();

protected:
	// 구르기 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	class UAnimMontage* RollMontage;

	// 구르기 거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float RollDistance = 400.0f;

	// 구르기 속도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float RollSpeed = 800.0f;

	// 구르기 중 무적 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float InvincibilityFrames = 0.5f;

	// 구르기 후 회복 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float RecoveryTime = 0.3f;

public:
	// 어빌리티 활성화
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 어빌리티 종료
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 구르기 실행
	UFUNCTION(BlueprintCallable, Category = "Roll")
	virtual void PerformRoll();

	// 구르기 방향 계산
	UFUNCTION(BlueprintPure, Category = "Roll")
	virtual FVector CalculateRollDirection() const;

	// 무적 상태 시작
	UFUNCTION(BlueprintCallable, Category = "Roll")
	virtual void StartInvincibility();

	// 무적 상태 종료
	UFUNCTION(BlueprintCallable, Category = "Roll")
	virtual void EndInvincibility();

private:
	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 무적 상태 타이머
	FTimerHandle InvincibilityTimer;

	// 몽타주 종료 델리게이트 핸들
	FOnMontageEnded MontageEndedDelegate;

	// 구르기 시작 시간
	float RollStartTime = 0.0f;
};