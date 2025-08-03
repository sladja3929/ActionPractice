// Fill out your copyright notice in the Description page of Project Settings.


#include "Notifies/AnimNotify_AttackRecoveryEnd.h"
#include "Characters/ActionPracticeCharacter.h"

void UAnimNotify_AttackRecoveryEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (AActionPracticeCharacter* Character = Cast<AActionPracticeCharacter>(MeshComp->GetOwner()))
		{
			Character->AttackRecoveryEnd();
			Character->CheckComboInput();
		}
	}
}
