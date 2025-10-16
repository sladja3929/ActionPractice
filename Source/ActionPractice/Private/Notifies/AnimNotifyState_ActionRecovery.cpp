#include "Notifies/AnimNotifyState_ActionRecovery.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayTagAssetInterface.h"
#include "AbilitySystemComponent.h"
#include "GAS/GameplayTagsDataAsset.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/GameplayTagsSubsystem.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogAnimNotifyState_ActionRecovery, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogAnimNotifyState_ActionRecovery, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

UAnimNotifyState_ActionRecovery::UAnimNotifyState_ActionRecovery()
{
    
}

void UAnimNotifyState_ActionRecovery::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
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
    EventData.EventTag = UGameplayTagsSubsystem::GetEventNotifyActionRecoveryStartTag();

    // Duration을 EventMagnitude에 저장
    EventData.EventMagnitude = TotalDuration;

    ASC->HandleGameplayEvent(UGameplayTagsSubsystem::GetEventNotifyActionRecoveryStartTag(), &EventData);
    DEBUG_LOG(TEXT("ActionRecovery ANS: Start"));
}

void UAnimNotifyState_ActionRecovery::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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
    DEBUG_LOG(TEXT("ActionRecovery ANS: End"));
}