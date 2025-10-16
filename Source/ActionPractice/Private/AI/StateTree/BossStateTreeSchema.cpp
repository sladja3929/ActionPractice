#include "AI/StateTree/BossStateTreeSchema.h"
#include "StateTreeTypes.h"
#include "StateTreeConditionBase.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeTaskBase.h"
#include "AI/BossAIController.h"
#include "Characters/BossCharacter.h"
#include "GAS/AbilitySystemComponent/BossAbilitySystemComponent.h"

#define ENABLE_DEBUG_LOG 1
#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogBossStateTreeSchema, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogBossStateTreeSchema, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

bool UBossStateTreeSchema::IsStructAllowed(const UScriptStruct* InScriptStruct) const
{
	//Task, Evaluator, Condition 허용
	return InScriptStruct->IsChildOf(FStateTreeTaskBase::StaticStruct()) ||
		   InScriptStruct->IsChildOf(FStateTreeEvaluatorBase::StaticStruct()) ||
		   InScriptStruct->IsChildOf(FStateTreeConditionBase::StaticStruct());
}

bool UBossStateTreeSchema::IsClassAllowed(const UClass* InClass) const
{
	//BossAIController, BossCharacter, Actor 허용
	return InClass->IsChildOf(ABossAIController::StaticClass()) ||
		   InClass->IsChildOf(ABossCharacter::StaticClass()) ||
		   InClass->IsChildOf(AActor::StaticClass());
}

bool UBossStateTreeSchema::IsExternalItemAllowed(const UStruct& InStruct) const
{
	return true;
}

//언리얼 5.6부터는 Context 추가를 GetContextDataDescs에서 오버라이드하여 사용
TConstArrayView<FStateTreeExternalDataDesc> UBossStateTreeSchema::GetContextDataDescs() const
{
	//Static으로 Context 데이터를 생성하여 메모리 유지 (일반 변수는 에셋 생성 시 에러남)
	static TArray<FStateTreeExternalDataDesc> CachedDescs;

	//매번 새로 생성 (부모 클래스의 Context가 바뀔 수 있으므로)
	CachedDescs.Reset();

	//부모 클래스의 Context 추가 (AIController 등)
	TConstArrayView<FStateTreeExternalDataDesc> ParentDescsView = Super::GetContextDataDescs();
	CachedDescs.Append(ParentDescsView.GetData(), ParentDescsView.Num());

	// 커스텀 Context 데이터 추가
	// GAS 사용을 위한 ASC
	CachedDescs.Add(FStateTreeExternalDataDesc(UBossAbilitySystemComponent::StaticClass(), EStateTreeExternalDataRequirement::Required));

	return CachedDescs;
}