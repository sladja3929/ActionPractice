#pragma once

#include "CoreMinimal.h"
#include "MontageAbilityInterface.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "ActionRecoveryAbility.generated.h"

class UAbilityTask_WaitDelay;
class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;

/***
 * 몽타주를 사용하며 ActionRecovery와 RotateCharacter가 있는 어빌리티.
 */
UCLASS(Abstract)
class ACTIONPRACTICE_API UActionRecoveryAbility : public UActionPracticeGameplayAbility, public IMontageAbilityInterface
{
	GENERATED_BODY()

public:
#pragma region "Public Functions"
	UActionRecoveryAbility();

#pragma endregion

protected:
#pragma region "Protected Variables"
	// 캐릭터 회전 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	float RotateTime = 0.1f;

	// 몽타주 재생 태스크
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = nullptr;

	// 딜레이 태스크
	UPROPERTY()
	UAbilityTask_WaitDelay* WaitDelayTask = nullptr;
	
	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitPlayBufferEventTask = nullptr;
#pragma endregion

#pragma region "Protected Functions"

	UFUNCTION()
	virtual void ConsumeStaminaAndAddTag();

	UFUNCTION()
	virtual void RotateCharacter();
	
	UFUNCTION()
	virtual void PlayAction() override;

	UFUNCTION()
	virtual void ExecuteMontageTask() override PURE_VIRTUAL(UMontageAbility::ExecuteMontageTask);

	UFUNCTION()
	virtual void BindAndReadyPlayBufferEvent();

	// 몽타주 태스크 완료 콜백
	UFUNCTION()
	virtual void OnTaskMontageCompleted() override;

	// 몽타주 태스크 중단 콜백
	UFUNCTION()
	virtual void OnTaskMontageInterrupted() override;

	//입력 버퍼 실행 콜백
	UFUNCTION()
	virtual void OnEventPlayBuffer(FGameplayEventData Payload) {}
#pragma endregion

private:
#pragma region "Private Variables"
#pragma endregion

#pragma region "Private Functions"
#pragma endregion
};