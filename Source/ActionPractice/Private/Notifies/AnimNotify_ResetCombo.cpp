#include "Notifies/AnimNotify_ResetCombo.h"
#include "Characters/ActionPracticeCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"
#include "GAS/GameplayTagsSubsystem.h"

void UAnimNotify_ResetCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			MeshComp->GetOwner(), 
			UGameplayTagsSubsystem::GetEventNotifyResetComboTag(), 
			FGameplayEventData()
		);
	}
}