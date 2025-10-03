#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputBufferComponent.generated.h"

class AActionPracticeCharacter;
class UInputAction;
class UActionPracticeGameplayAbility;
class UGameplayAbility;
struct FGameplayEventData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ACTIONPRACTICE_API UInputBufferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"

	UPROPERTY()
	bool bCanBufferInput = false;

	UPROPERTY()
	bool bBufferActionReleased = false;
#pragma endregion
	
#pragma region "Public Functions"

	UInputBufferComponent();

	UFUNCTION()
	void BufferNextAction(const UInputAction* InputedAction);

	UFUNCTION()
	void UnBufferHoldAction(const UInputAction* InputedAction);

	UFUNCTION()
	bool IsBufferWaiting();

#pragma endregion

protected:
#pragma region "Protected Variables"

	UPROPERTY()
	const UInputAction* BufferedAction = nullptr;

	UPROPERTY()
	int32 CurrentBufferPriority = -1;

	UPROPERTY()
	TSet<const UInputAction*> BufferedHoldAction;

	FDelegateHandle EnableBufferInputHandle;
	FDelegateHandle ActionRecoveryStartHandle;
	FDelegateHandle ActionRecoveryEndHandle;
	
#pragma endregion

#pragma region "Protected Functions"

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION()
	void ActivateBufferAction();

	//어빌리티 몽타주 Start에서 이벤트 호출 (추후 노티파이 추가 가능)
	UFUNCTION()
	void OnActionRecoveryStart(const FGameplayEventData& EventData);
	
	//노티파이를 부착하거나, 어빌리티 몽타주 Start에서 이벤트 호출
	UFUNCTION()
	void OnEnableBufferInput(const FGameplayEventData& EventData);

	//노티파이를 부착하거나, 어빌리티 몽타주 End에서 이벤트 호출
	UFUNCTION()
	void OnActionRecoveryEnd(const FGameplayEventData& EventData);
	
#pragma endregion

private:
#pragma region "Private Variables"
	
	UPROPERTY()
	TObjectPtr<AActionPracticeCharacter> OwnerCharacter = nullptr;
	
#pragma endregion
	
#pragma region "Private Functions"
	// 인풋액션으로 해당 어빌리티의 버퍼 가능 여부와 우선순위 확인
	UFUNCTION()
	bool CanBufferAction(const UInputAction* InputAction, int32& OutPriority, bool& bIsHoldAction) const;

	UFUNCTION()
	void ActivateAbility(const UInputAction* InputAction);
#pragma endregion

};