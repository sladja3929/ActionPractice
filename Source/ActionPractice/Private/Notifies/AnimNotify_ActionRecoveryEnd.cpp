// Fill out your copyright notice in the Description page of Project Settings.


#include "Notifies/AnimNotify_ActionRecoveryEnd.h"
#include "Characters/ActionPracticeCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "GAS/GameplayTagsSubsystem.h"

void UAnimNotify_ActionRecoveryEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			MeshComp->GetOwner(), 
			UGameplayTagsSubsystem::GetEventNotifyActionRecoveryEndTag(), 
			FGameplayEventData()
		);
	}
}
