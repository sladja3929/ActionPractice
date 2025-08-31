#include "Notifies/AnimNotify_InvincibleStart.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/GameplayTagsSubsystem.h"

void UAnimNotify_InvincibleStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
				MeshComp->GetOwner(),
				UGameplayTagsSubsystem::GetEventNotifyInvincibleStartTag(), 
				FGameplayEventData()
			);
		}
	}
}