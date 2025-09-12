#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AnimNotifyState_HitDetection.generated.h"

UCLASS(meta = (DisplayName = "Hit Detection Event"))
class ACTIONPRACTICE_API UAnimNotifyState_HitDetection : public UAnimNotifyState
{
    GENERATED_BODY()

public:
#pragma region "Public Functions"
    UAnimNotifyState_HitDetection();
    
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
    
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
#pragma endregion

#pragma region "Public Variables"
    
#pragma endregion
};