#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_InvincibleStart.generated.h"

UCLASS(meta = (DisplayName = "Invincible Start"))
class ACTIONPRACTICE_API UAnimNotify_InvincibleStart : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("Invincible Start");
	}
};