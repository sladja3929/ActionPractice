// Fill out your copyright notice in the Description page of Project Settings.


#include "Notifies/AnimNotify_EnableComboInput.h"
#include "Characters/ActionPracticeCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"

void UAnimNotify_EnableComboInput::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Montage.EnableComboInput"));
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, FGameplayEventData());
	}
}