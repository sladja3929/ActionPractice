#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AnimNotifyState_ActionRecovery.generated.h"

UCLASS(meta = (DisplayName = "Action Recovery Event"))
class ACTIONPRACTICE_API UAnimNotifyState_ActionRecovery : public UAnimNotifyState
{
    GENERATED_BODY()

public:
#pragma region "Public Functions"
    UAnimNotifyState_ActionRecovery();

	//주의!! ANS는 AN보다 한 프레임 늦게 트리거되기 때문에 빈 틈이 생길 수 있음
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
#pragma endregion

#pragma region "Public Variables"

#pragma endregion
};