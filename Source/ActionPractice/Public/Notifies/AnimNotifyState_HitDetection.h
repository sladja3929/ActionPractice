#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "AnimNotifyState_HitDetection.generated.h"

/**
 * 순수 이벤트 트리거 역할의 Weapon Trace 노티파이
 * Component 참조 없이 GameplayTag 이벤트만 발생
 * 플레이어, NPC, 몬스터 모두 사용 가능
 */
UCLASS(meta = (DisplayName = "Weapon Trace Event"))
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