#include "Notifies/AnimNotify_ResetCombo.h"
#include "Characters/ActionPracticeCharacter.h"

void UAnimNotify_ResetCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		if (AActionPracticeCharacter* Character = Cast<AActionPracticeCharacter>(MeshComp->GetOwner()))
		{
			Character->ResetCombo();
		}
	}
}