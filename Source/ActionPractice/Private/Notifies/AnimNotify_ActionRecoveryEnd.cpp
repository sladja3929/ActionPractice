// Fill out your copyright notice in the Description page of Project Settings.


#include "Notifies/AnimNotify_ActionRecoveryEnd.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GAS/GameplayTagsSubsystem.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogAnimNotify_ActionRecoveryEnd, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogAnimNotify_ActionRecoveryEnd, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

void UAnimNotify_ActionRecoveryEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner())
		return;

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner))
		return;

	UAbilitySystemComponent* ASC = Owner->FindComponentByClass<UAbilitySystemComponent>();
	if (!ASC || !IsValid(ASC))
		return;

	FGameplayEventData EventData;
	EventData.Instigator = Owner;
	EventData.Target = Owner;
	EventData.EventTag = UGameplayTagsSubsystem::GetEventNotifyActionRecoveryEndTag();

	ASC->HandleGameplayEvent(UGameplayTagsSubsystem::GetEventNotifyActionRecoveryEndTag(), &EventData);
	DEBUG_LOG(TEXT("ActionRecoveryEnd AN"));
}
