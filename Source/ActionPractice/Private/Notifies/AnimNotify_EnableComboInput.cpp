// Fill out your copyright notice in the Description page of Project Settings.


#include "Notifies/AnimNotify_EnableComboInput.h"
#include "Characters/ActionPracticeCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "GAS/GameplayTagsSubsystem.h"

void UAnimNotify_EnableComboInput::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			MeshComp->GetOwner(), 
			UGameplayTagsSubsystem::GetEventNotifyEnableComboInputTag(), 
			FGameplayEventData()
		);
	}
}