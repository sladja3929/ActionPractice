// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_EnableBufferInput.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Enable Buffer Input"))
class ACTIONPRACTICE_API UAnimNotify_EnableBufferInput : public UAnimNotify
{
	GENERATED_BODY()

public:
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("Enable Buffer Input");
	}
};