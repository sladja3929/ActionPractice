#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ChargeStart.generated.h"

UCLASS(meta = (DisplayName = "Charge Start"))
class ACTIONPRACTICE_API UAnimNotify_ChargeStart : public UAnimNotify
{
	GENERATED_BODY()

public:
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("Charge Start");
	}
};