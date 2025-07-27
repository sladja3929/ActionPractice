// Fill out your copyright notice in the Description page of Project Settings.


#include "Notifies/AnimNotify_DisableComboInput.h"
#include "Characters/ActionPracticeCharacter.h"

void UAnimNotify_DisableComboInput::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (AActionPracticeCharacter* Character = Cast<AActionPracticeCharacter>(MeshComp->GetOwner()))
		{
			Character->DisableComboInput();
		}
	}
}