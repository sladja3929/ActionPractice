// Fill out your copyright notice in the Description page of Project Settings.


#include "Notifies/AnimNotify_AttackRecoveryEnd.h"
#include "Characters/ActionPracticeCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"

void UAnimNotify_AttackRecoveryEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Montage.AttackRecoveryEnd"));
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, FGameplayEventData());
	}
}
