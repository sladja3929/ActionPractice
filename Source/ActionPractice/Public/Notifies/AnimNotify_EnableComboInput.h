// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_EnableComboInput.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Enable Combo Input"))
class ACTIONPRACTICE_API UAnimNotify_EnableComboInput : public UAnimNotify
{
	GENERATED_BODY()

public:
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("Enable Combo Input");
	}
};