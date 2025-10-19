#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class AActionPracticeCharacter;
class UStateTreeAIComponent;
class UGASStateTreeAIComponent;
class ABossCharacter;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class ACTIONPRACTICE_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"

	TWeakObjectPtr<AActionPracticeCharacter> DetectedPlayer = nullptr;
	
#pragma endregion

#pragma region "Public Functions"

	AEnemyAIController();

	FORCEINLINE UGASStateTreeAIComponent* GetStateTreeComponent() const { return GASStateTreeAIComponent; }
	FORCEINLINE ABossCharacter* GetBossCharacter() const { return BossCharacter.Get(); }
	
#pragma endregion

protected:
#pragma region "Protected Variables"

	//State Tree 에셋을 브레인으로 설정하고 실행하는 컴포넌트
	//5.6부터는 스키마와 컴포넌트가 일치해야 함(기본 컴포넌트는 커스텀 스키마 에셋이 에셋 피커에서 뜨지 않음, 버그일수도)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UGASStateTreeAIComponent> GASStateTreeAIComponent;

#pragma endregion

#pragma region "Protected Functions"

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	//Perception 이벤트 핸들러
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

#pragma endregion

private:
#pragma region "Private Variables"

	TWeakObjectPtr<ABossCharacter> BossCharacter;

#pragma endregion

#pragma region "Private Functions"

#pragma endregion
};