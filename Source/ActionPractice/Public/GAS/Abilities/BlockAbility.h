#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "BlockAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API UBlockAbility : public UActionPracticeGameplayAbility
{
	GENERATED_BODY()

public:
	UBlockAbility();

protected:
	// 방어 시작 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	class UAnimMontage* BlockStartMontage;

	// 방어 유지 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	class UAnimMontage* BlockIdleMontage;

	// 방어 종료 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	class UAnimMontage* BlockEndMontage;

	// 방어 성공 몽타주 (패리 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	class UAnimMontage* BlockSuccessMontage;

	// 방어 실패 몽타주 (가드 브레이크)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	class UAnimMontage* BlockFailMontage;

	// 스태미나 소모량 (초당)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	float StaminaDrainPerSecond = 5.0f;

	// 방어력 배율 (받는 데미지 감소)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	float DamageReductionMultiplier = 0.8f; // 20% 데미지만 받음

	// 스태미나 데미지 감소율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	float StaminaDamageReduction = 0.5f; // 50% 스태미나 데미지만 받음

	// 이동 속도 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	float MovementSpeedMultiplier = 0.3f; // 30%로 속도 감소

	// 방어 각도 (정면 기준)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	float BlockAngle = 120.0f; // 정면 120도

	// 패리 윈도우 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	float ParryWindow = 0.5f;

	// 가드 브레이크 임계값
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Block")
	float GuardBreakThreshold = 50.0f;

public:
	// 어빌리티 활성화
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	// 어빌리티 종료
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 방어 시작
	UFUNCTION(BlueprintCallable, Category = "Block")
	virtual void StartBlocking();

	// 방어 중 처리
	UFUNCTION(BlueprintCallable, Category = "Block")
	virtual void HandleBlocking(float DeltaTime);

	// 방어 종료
	UFUNCTION(BlueprintCallable, Category = "Block")
	virtual void StopBlocking();

	// 방어 성공 처리
	UFUNCTION(BlueprintCallable, Category = "Block")
	virtual void HandleBlockSuccess(float IncomingDamage, AActor* DamageSource);

	// 패리 성공 처리
	UFUNCTION(BlueprintCallable, Category = "Block")
	virtual void HandleParrySuccess(AActor* DamageSource);

	// 가드 브레이크 처리
	UFUNCTION(BlueprintCallable, Category = "Block")
	virtual void HandleGuardBreak();

	// 방어 방향 확인
	UFUNCTION(BlueprintPure, Category = "Block")
	virtual bool CanBlockFromDirection(const FVector& DamageDirection) const;

	// 방어력 계산
	UFUNCTION(BlueprintPure, Category = "Block")
	virtual float CalculateBlockEffectiveness() const;

	// 패리 윈도우 체크
	UFUNCTION(BlueprintPure, Category = "Block")
	virtual bool IsInParryWindow() const;

private:
	// 방어 시작 시간
	UPROPERTY()
	float BlockStartTime = 0.0f;

	// 방어 상태
	UPROPERTY()
	bool bIsBlocking = false;

	// 스태미나 타이머
	FTimerHandle StaminaDrainTimer;

	// 스태미나 소모 함수
	UFUNCTION()
	void DrainStamina();

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 몽타주 종료 델리게이트 핸들
	FOnMontageEnded MontageEndedDelegate;
};