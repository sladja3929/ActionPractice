#include "AI/StateTree/GASStateTreeAIComponentSchema.h"
#include "StateTreeTypes.h"
#include "StateTreeConditionBase.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeTaskBase.h"
#include "AI/EnemyAIController.h"
#include "Characters/BossCharacter.h"
#include "GAS/AbilitySystemComponent/BossAbilitySystemComponent.h"
#include "GAS/AttributeSet/BossAttributeSet.h"


#define ENABLE_DEBUG_LOG 1
#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogBossStateTreeSchema, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogBossStateTreeSchema, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

bool UGASStateTreeAIComponentSchema::IsStructAllowed(const UScriptStruct* InScriptStruct) const
{
	//Task, Evaluator, Condition 허용
	return InScriptStruct->IsChildOf(FStateTreeTaskBase::StaticStruct()) ||
		   InScriptStruct->IsChildOf(FStateTreeEvaluatorBase::StaticStruct()) ||
		   InScriptStruct->IsChildOf(FStateTreeConditionBase::StaticStruct());
}

bool UGASStateTreeAIComponentSchema::IsClassAllowed(const UClass* InClass) const
{
	//BossAIController, BossCharacter, Actor 허용
	return InClass->IsChildOf(AEnemyAIController::StaticClass()) ||
		   InClass->IsChildOf(ABossCharacter::StaticClass()) ||
		   InClass->IsChildOf(AActor::StaticClass());
}

bool UGASStateTreeAIComponentSchema::IsExternalItemAllowed(const UStruct& InStruct) const
{
	return true;
}

//언리얼 5.6부터는 GetContextDataDescs로 Context 배열을 가져온 후 Context를 추가
TConstArrayView<FStateTreeExternalDataDesc> UGASStateTreeAIComponentSchema::GetContextDataDescs() const
{
	//Static으로 Context 데이터 배열를 가져와서 메모리 유지 (일반 변수는 에셋 생성 시 에러남)
	static TArray<FStateTreeExternalDataDesc> CachedDescs;

	//매번 새로 생성 (부모 클래스의 Context가 바뀔 수 있으므로)
	CachedDescs.Reset();

	//부모 클래스의 기존 Context 그대로 가져오기 (Actor, AIController)
	TConstArrayView<FStateTreeExternalDataDesc> ParentDescsView = Super::GetContextDataDescs();
	CachedDescs.Append(ParentDescsView.GetData(), ParentDescsView.Num());

	//커스텀 Context 데이터 추가
	
	//ASC
	FStateTreeExternalDataDesc ASCDesc(UBossAbilitySystemComponent::StaticClass(), EStateTreeExternalDataRequirement::Optional);
	ASCDesc.Name = TEXT("AbilitySystemComponent");
	CachedDescs.Add(ASCDesc);
	
	//AttributeSet
	FStateTreeExternalDataDesc AttributeSetDesc(UBossAttributeSet::StaticClass(), EStateTreeExternalDataRequirement::Optional);
	AttributeSetDesc.Name = TEXT("AttributeSet");
	CachedDescs.Add(AttributeSetDesc);
	
	return CachedDescs;
}