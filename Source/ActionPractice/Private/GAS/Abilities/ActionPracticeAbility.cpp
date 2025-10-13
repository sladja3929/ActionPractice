#include "GAS/Abilities/ActionPracticeAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/AttributeSet/ActionPracticeAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GAS/AbilitySystemComponent/ActionPracticeAbilitySystemComponent.h"
#include "GAS/GameplayTagsSubsystem.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogActionPracticeGameplayAbility, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogActionPracticeGameplayAbility, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

UActionPracticeAbility::UActionPracticeAbility()
{
	// 기본 설정
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

//Ability는 OnGiveAbility가 BeginPlay처럼 사용됨
void UActionPracticeAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	EffectStaminaCostTag = UGameplayTagsSubsystem::GetEffectStaminaCostTag();
	if (!EffectStaminaCostTag.IsValid())
	{
		DEBUG_LOG(TEXT("EffectStaminaCostTag is Invalid"));
	}
}

bool UActionPracticeAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		DEBUG_LOG(TEXT("Cannot Activate Ability"));
		return false;
	}

	//초기 스테미나 소모값 확인
	if (!CheckStaminaCost(*ActorInfo))
	{
		DEBUG_LOG(TEXT("Cannot Activate Ability"));
		return false;
	}

	return true;
}

void UActionPracticeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		DEBUG_LOG(TEXT("Cannot Commit Ability"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	ActivateInitSettings();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UActionPracticeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UActionPracticeAbility::CheckStaminaCost(const FGameplayAbilityActorInfo& ActorInfo) const
{
	//0이면 스테미나 소모는 없지만 현재 스테미나가 있는지는 확인, 음수면 체크도 하지 않음
	if (StaminaCost < 0.0f)
	{
		return true;
	}
	
	if (UActionPracticeAttributeSet* AttributeSet = GetActionPracticeAttributeSetFromActorInfo(&ActorInfo))
	{
		return AttributeSet->GetStamina() > 0;
	}

	return false;
}

bool UActionPracticeAbility::ApplyStaminaCost()
{
	//0이면 스테미나 소모는 없지만 현재 스테미나가 있는지는 확인, 음수면 체크도 하지 않음
	if (StaminaCost < 0.0f)
	{
		return true;
	}

	UActionPracticeAttributeSet* AttributeSet = GetActionPracticeAttributeSetFromActorInfo();
	if (!AttributeSet || AttributeSet->GetStamina() < 3.0f)
	{
		DEBUG_LOG(TEXT("No AttributeSet or Low Stamina"));
		return false;
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

void UActionPracticeAbility::SetStaminaCost(float InStaminaCost)
{
	StaminaCost = InStaminaCost;
}

UActionPracticeAbilitySystemComponent* UActionPracticeAbility::GetActionPracticeAbilitySystemComponentFromActorInfo() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		return Cast<UActionPracticeAbilitySystemComponent>(ASC);
	}
	return nullptr;
}

AActionPracticeCharacter* UActionPracticeAbility::GetActionPracticeCharacterFromActorInfo() const
{
	return Cast<AActionPracticeCharacter>(GetActorInfo().AvatarActor.Get());
}

AActionPracticeCharacter* UActionPracticeAbility::GetActionPracticeCharacterFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const
{
	return Cast<AActionPracticeCharacter>(ActorInfo->AvatarActor.Get());
}

UActionPracticeAttributeSet* UActionPracticeAbility::GetActionPracticeAttributeSetFromActorInfo() const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (Character)
	{
		return Character->GetAttributeSet();
	}
	return nullptr;
}

UActionPracticeAttributeSet* UActionPracticeAbility::GetActionPracticeAttributeSetFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo(ActorInfo);
	if (Character)
	{
		return Character->GetAttributeSet();
	}
	return nullptr;
}

UInputBufferComponent* UActionPracticeAbility::GetInputBufferComponentFromActorInfo() const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (Character)
	{
		return Character->GetInputBufferComponent();
	}
	return nullptr;
}
