// Fill out your copyright notice in the Description page of Project Settings.


#include "Notifies/AnimNotify_EnableComboInput.h"
#include "Characters/ActionPracticeCharacter.h"

void UAnimNotify_EnableComboInput::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (AActionPracticeCharacter* Character = Cast<AActionPracticeCharacter>(MeshComp->GetOwner()))
		{
			Character->EnableComboInput();
		}
	}
}