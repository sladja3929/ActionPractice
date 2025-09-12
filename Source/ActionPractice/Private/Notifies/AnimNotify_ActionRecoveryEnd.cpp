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
		// AbilitySystemComponent가 있는 액터에서만 이벤트 전송 (애니메이션 에디터 프리뷰 액터 제외)
		if (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				MeshComp->GetOwner(), 
				UGameplayTagsSubsystem::GetEventNotifyActionRecoveryEndTag(), 
				FGameplayEventData()
			);
		}
	}
}
