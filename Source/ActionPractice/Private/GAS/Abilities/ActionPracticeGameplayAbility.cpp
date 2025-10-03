#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GAS/ActionPracticeAbilitySystemComponent.h"
#include "GAS/GameplayTagsSubsystem.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogActionPracticeGameplayAbility, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogActionPracticeGameplayAbility, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

UActionPracticeGameplayAbility::UActionPracticeGameplayAbility()
{
	// 기본 설정
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

//Ability는 OnGiveAbility가 BeginPlay처럼 사용됨
void UActionPracticeGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	EffectStaminaCostTag = UGameplayTagsSubsystem::GetEffectStaminaCostTag();
	if (!EffectStaminaCostTag.IsValid())
	{
		DEBUG_LOG(TEXT("EffectStaminaCostTag is Invalid"));
	}
}

bool UActionPracticeGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	//초기 스테미나 소모값 확인
	if (!CheckStaminaCost(*ActorInfo))
	{
		return false;
	}

	return true;
}

void UActionPracticeGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!ApplyStaminaCost())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UActionPracticeGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UActionPracticeGameplayAbility::CheckStaminaCost(const FGameplayAbilityActorInfo& ActorInfo) const
{
	if (StaminaCost <= 0.0f)
	{
		return true;
	}
	
	if (UActionPracticeAttributeSet* AttributeSet = GetActionPracticeAttributeSetFromActorInfo(&ActorInfo))
	{
		return AttributeSet->GetStamina() > 0;
	}

	return false;
}

bool UActionPracticeGameplayAbility::ApplyStaminaCost()
{
	if (StaminaCost <= 0.0f)
	{
		return true;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !StaminaCostEffect)
	{
		DEBUG_LOG(TEXT("No ASC or StaminaCostEffect"));
		return false;
	}
	
	//EffectSpec 생성
	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	const float EffectiveLevel = static_cast<float>(GetAbilityLevel());
	FGameplayEffectSpecHandle EffectSpec = ASC->MakeOutgoingSpec(StaminaCostEffect, EffectiveLevel, EffectContext);
	
	if (!EffectSpec.IsValid())
	{
		DEBUG_LOG(TEXT("Failed StaminaCost GameplayEffectSpec"));
		return false;
	}

	EffectSpec.Data.Get()->SetSetByCallerMagnitude(EffectStaminaCostTag, -StaminaCost);
	const FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
	const bool bApplied = Handle.IsValid();
	
	DEBUG_LOG(TEXT("ApplyStaminaCost applied=%s, Cost=%.2f"), bApplied ? TEXT("true") : TEXT("false"), StaminaCost);

	return true;
}

void UActionPracticeGameplayAbility::SetStaminaCost(float InStaminaCost, float InStaminaRegenBlockDuration)
{
	StaminaCost = InStaminaCost;
	StaminaRegenBlockDuration = InStaminaRegenBlockDuration;
}

UActionPracticeAbilitySystemComponent* UActionPracticeGameplayAbility::GetActionPracticeAbilitySystemComponentFromActorInfo() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		return Cast<UActionPracticeAbilitySystemComponent>(ASC);
	}
	return nullptr;
}

AActionPracticeCharacter* UActionPracticeGameplayAbility::GetActionPracticeCharacterFromActorInfo() const
{
	return Cast<AActionPracticeCharacter>(GetActorInfo().AvatarActor.Get());
}

AActionPracticeCharacter* UActionPracticeGameplayAbility::GetActionPracticeCharacterFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return Cast<AActionPracticeCharacter>(ActorInfo->AvatarActor.Get());
}

UActionPracticeAttributeSet* UActionPracticeGameplayAbility::GetActionPracticeAttributeSetFromActorInfo() const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (Character)
	{
		return Character->GetAttributeSet();
	}
	return nullptr;
}

UActionPracticeAttributeSet* UActionPracticeGameplayAbility::GetActionPracticeAttributeSetFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo(ActorInfo);
	if (Character)
	{
		return Character->GetAttributeSet();
	}
	return nullptr;
}

UInputBufferComponent* UActionPracticeGameplayAbility::GetInputBufferComponentFromActorInfo() const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (Character)
	{
		return Character->GetInputBufferComponent();
	}
	return nullptr;
}
