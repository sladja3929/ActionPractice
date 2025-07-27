#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ResetCombo.generated.h"

UCLASS(meta = (DisplayName = "Reset Combo"))
class ACTIONPRACTICE_API UAnimNotify_ResetCombo : public UAnimNotify
{
	GENERATED_BODY()

public:
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("Reset Combo");
	}
};