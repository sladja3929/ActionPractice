#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "ShortDurationTagManager.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct FShortDurationTagInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag Tag;

	UPROPERTY()
	float EndTime;

	UPROPERTY()
	bool bIsStacking;

	FShortDurationTagInfo()
	{
		EndTime = 0.0f;
		bIsStacking = false;
	}
};

/**
 * 1초 미만의 짧은 Duration을 지원하는 게임플레이 태그 관리 클래스
 * GE의 1초 제한을 우회하기 위해 타이머 기반으로 작동
 * 소유한 ASC에서 사용되어야 함, 외부
 */

UCLASS(BlueprintType)
class ACTIONPRACTICE_API UShortDurationTagManager : public UObject
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"
	
#pragma endregion

#pragma region "Public Functions"
	
	UShortDurationTagManager();

	//태그 적용
	UFUNCTION(BlueprintCallable, Category = "Short Duration Tag")
	void ApplyTag(
		UAbilitySystemComponent* ASC,
		const FGameplayTag& Tag,
		float Duration,
		bool bIsStack = false
	);

	//태그 제거
	UFUNCTION(BlueprintCallable, Category = "Short Duration Tag")
	void RemoveTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag);

	//특정 태그의 남은 시간 확인
	UFUNCTION(BlueprintPure, Category = "Short Duration Tag")
	float GetRemainingTime(const FGameplayTag& Tag) const;

	//모든 태그 제거
	UFUNCTION(BlueprintCallable, Category = "Short Duration Tag")
	void RemoveAllTags();

	//초기화
	void Initialize(UAbilitySystemComponent* InOwnerASC);

	//정리
	void Cleanup();

#pragma endregion

protected:
#pragma region "Protected Variables"
	
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> OwnerASC;
	
	UPROPERTY()
	TMap<FGameplayTag, FShortDurationTagInfo> ActiveTags;
	
	FTimerHandle UpdateTimerHandle;

	//업데이트 주기 (초)
	float UpdateInterval = 0.05f;

#pragma endregion

#pragma region "Protected Functions"

	//타이머 업데이트 함수
	UFUNCTION()
	void UpdateTags();

	//태그 제거 처리
	void RemoveTagInternal(const FGameplayTag& Tag);

	//타이머 시작/정지
	void StartUpdateTimer();
	void StopUpdateTimer();

#pragma endregion

private:
#pragma region "Private Variables"
	
#pragma endregion

#pragma region "Private Functions"
	
#pragma endregion
};