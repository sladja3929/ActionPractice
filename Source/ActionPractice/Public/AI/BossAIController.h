#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BossAIController.generated.h"

class UStateTreeAIComponent;
class UStateTree;
class ABossCharacter;

UCLASS()
class ACTIONPRACTICE_API ABossAIController : public AAIController
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"

#pragma endregion

#pragma region "Public Functions"

	ABossAIController();

	FORCEINLINE UStateTreeAIComponent* GetStateTreeComponent() const { return StateTreeAIComponent; }
	FORCEINLINE ABossCharacter* GetBossCharacter() const { return BossCharacter.Get(); }

#pragma endregion

protected:
#pragma region "Protected Variables"

	//State Tree 에셋을 브레인으로 설정하고 실행하는 컴포넌트
	//추가적인 Tree 기능 구현이 아니라면 커스텀 자식을 만들 필요 없음
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeAIComponent;

	//콘텐츠 브라우저에서 생성 후 할당, Schema를 베이스로 하여 State Tree의 전체 상태 머신 구조를 가짐
	//스키마는 Asset 내부 설정만 바꾸기 때문에 스키마에 상관 없이 UStateTree 그대로 사용
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UStateTree> StateTreeAsset;

#pragma endregion

#pragma region "Protected Functions"

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

#pragma endregion

private:
#pragma region "Private Variables"

	TWeakObjectPtr<ABossCharacter> BossCharacter;

#pragma endregion

#pragma region "Private Functions"

#pragma endregion
};