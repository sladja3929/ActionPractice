#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeAIComponent.h"
#include "GASStateTreeAIComponentSchema.h" // StaticClass 사용을 위해 포함
#include "GASStateTreeAIComponent.generated.h"

/**
 * GAS 통합을 위한 커스텀 StateTree AI Component
 * - GetSchema 오버라이드만으로 커스텀 스키마 지정
 */
UCLASS(ClassGroup = AI, Blueprintable, meta = (BlueprintSpawnableComponent))
class ACTIONPRACTICE_API UGASStateTreeAIComponent : public UStateTreeAIComponent
{
	GENERATED_BODY()

public:
	virtual TSubclassOf<UStateTreeSchema> GetSchema() const override;
};