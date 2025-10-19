#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "TestMoveTask.generated.h"

class AActor;

/**
 * Task Instance Data
 */
USTRUCT()
struct ACTIONPRACTICE_API FTestMoveTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> Actor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float Duration = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float Speed = 100.0f;

	float ElapsedTime = 0.0f;
};

/**
 * 액터를 앞으로 이동시키는 테스트 Task
 */
USTRUCT()
struct ACTIONPRACTICE_API FTestMoveTask : public FStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTestMoveTaskInstanceData;

	FTestMoveTask() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
