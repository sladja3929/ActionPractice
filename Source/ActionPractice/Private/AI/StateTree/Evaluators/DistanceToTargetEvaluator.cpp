// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/StateTree/Evaluators/DistanceToTargetEvaluator.h"
#include "AI/EnemyAIController.h"
#include "Characters/ActionPracticeCharacter.h"
#include "StateTreeExecutionContext.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogEnemyDistanceEvaluator, Log, All);
	#define DEBUG_LOG(Format, ...) UE_LOG(LogEnemyDistanceEvaluator, Warning, Format, ##__VA_ARGS__)
#else
	#define DEBUG_LOG(Format, ...)
#endif

void FDistanceToTargetEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.SourceActor)
	{
		DEBUG_LOG(TEXT("SourceActor is not valid in DistanceToTargetEvaluator"));
		return;
	}

	if (!InstanceData.AIController)
	{
		DEBUG_LOG(TEXT("AIController is not valid in DistanceToTargetEvaluator"));
		return;
	}

	DEBUG_LOG(TEXT("DistanceToTargetEvaluator TreeStart"));
}

void FDistanceToTargetEvaluator::TreeStop(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	InstanceData.DetectedTarget = nullptr;
	InstanceData.DistanceToTarget = -1.0f;
	InstanceData.bTargetDetected = false;

	DEBUG_LOG(TEXT("DistanceToTargetEvaluator TreeStop"));
}

void FDistanceToTargetEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	UpdateDistanceToTarget(Context);
}

void FDistanceToTargetEvaluator::UpdateDistanceToTarget(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!InstanceData.AIController || !InstanceData.AIController->DetectedPlayer.IsValid() || !InstanceData.SourceActor)
	{
		InstanceData.DetectedTarget = nullptr;
		InstanceData.DistanceToTarget = -1.0f;
		InstanceData.bTargetDetected = false;
		return;
	}

	InstanceData.DetectedTarget = InstanceData.AIController->GetDetectedPlayer();

	const FVector SourceLocation = InstanceData.SourceActor->GetActorLocation();
	const FVector TargetLocation = InstanceData.DetectedTarget->GetActorLocation();
	
	InstanceData.DistanceToTarget = FVector::Dist(SourceLocation, TargetLocation);
	InstanceData.bTargetDetected = true;
}
