#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_RotateToTarget.generated.h"

UCLASS(meta = (DisplayName = "Rotate To Target"))
class ACTIONPRACTICE_API UAnimNotify_RotateToTarget : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("Rotate To Target");
	}
};