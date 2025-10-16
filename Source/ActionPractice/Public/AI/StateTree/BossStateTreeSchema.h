#pragma once
#include "CoreMinimal.h"
#include "Components/StateTreeAIComponentSchema.h"
#include "BossStateTreeSchema.generated.h"

/*
 *BossAIController에서 사용하는 커스텀 State Tree Schema (기본 구조 설정)
 */

UCLASS(BlueprintType, EditInlineNew, CollapseCategories, meta = (DisplayName = "Boss AI Schema"))
class ACTIONPRACTICE_API UBossStateTreeSchema : public UStateTreeAIComponentSchema
{
	GENERATED_BODY()

public:
#pragma region "Public Functions"
	
	virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override;
	virtual bool IsClassAllowed(const UClass* InClass) const override;
	virtual bool IsExternalItemAllowed(const UStruct& InStruct) const override;
	virtual TConstArrayView<FStateTreeExternalDataDesc> GetContextDataDescs() const override;
	
#pragma endregion
};
