#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Public/Characters/ActionPracticeCharacter.h"
#include "InputBufferComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ACTIONPRACTICE_API UInputBufferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"

	UPROPERTY()
	bool bCanBufferInput;
	
#pragma endregion
	
#pragma region "Public Functions"

	UInputBufferComponent();

	//어빌리티의 EnableBufferInput에서 호출
	UFUNCTION()
	void BufferNextAction(UInputAction* InputedAction);

	//어빌리티의 OnMontageEnded나, ActionRecoveryEnd에서 호출
	UFUNCTION()
	void ActivateBufferAction();

#pragma endregion

protected:
#pragma region "Protected Variables"

	UInputAction* BufferedAction;
	
#pragma endregion

#pragma region "Protected Functions"

	virtual void BeginPlay() override;

#pragma endregion

private:
#pragma region "Private Variables"
	
	UPROPERTY()
	TObjectPtr<AActionPracticeCharacter> OwnerCharacter;
	
#pragma endregion
	
#pragma region "Private Functions"

#pragma endregion

};