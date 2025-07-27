// Fill out your copyright notice in the Description page of Project Settings.


#include "Notifies/AnimNotify_CheckComboInput.h"
#include "Characters/ActionPracticeCharacter.h"

void UAnimNotify_CheckComboInput::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (AActionPracticeCharacter* Character = Cast<AActionPracticeCharacter>(MeshComp->GetOwner()))
		{
			Character->CheckComboInput();
		}
	}
}
