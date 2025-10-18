#include "AI/StateTree/GASStateTreeAIComponent.h"

TSubclassOf<UStateTreeSchema> UGASStateTreeAIComponent::GetSchema() const
{
	return UGASStateTreeAIComponentSchema::StaticClass();
}