#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "MontageAbility.generated.h"

class UAbilityTask_WaitDelay;
class UAbilityTask_PlayMontageAndWait;

/***
 * 몽타주를 사용하여 ActionRecovery와 MontagePlay, RotateCharacter가 있는 어빌리티. 인터페이스 용도로 사용
 */
UCLASS(Abstract)
class ACTIONPRACTICE_API UMontageAbility : public UActionPracticeGameplayAbility
{
	GENERATED_BODY()

public:
#pragma region "Public Functions"
	UMontageAbility();

#pragma endregion

protected:
#pragma region "Protected Variables"
	// 캐릭터 회전 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	float RotateTime;

	// 몽타주 재생 태스크
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* PlayMontageTask;

	// 딜레이 태스크
	UPROPERTY()
	UAbilityTask_WaitDelay* WaitDelayTask;
#pragma endregion

#pragma region "Protected Functions"
	UFUNCTION()
	virtual void PlayMontage();

	UFUNCTION()
	virtual void ExecuteMontageTask() PURE_VIRTUAL(UMontageAbility::ExecuteMontageTask,);

	// 몽타주 태스크 완료 콜백
	UFUNCTION()
	virtual void OnTaskMontageCompleted();

	// 몽타주 태스크 중단 콜백
	UFUNCTION()
	virtual void OnTaskMontageInterrupted();
#pragma endregion

private:
#pragma region "Private Variables"
#pragma endregion

#pragma region "Private Functions"
#pragma endregion
};