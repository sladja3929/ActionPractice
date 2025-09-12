#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

UActionPracticeGameplayAbility::UActionPracticeGameplayAbility()
{
	// 기본 설정
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// 기본 태그 설정은 상속 클래스에서 직접 처리
}

void UActionPracticeGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	// 어빌리티가 부여될 때의 추가 설정이 필요하면 여기에 구현
}

bool UActionPracticeGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}
	
	// 스태미나 체크
	if (!CheckStaminaCost(*ActorInfo))
	{
		return false;
	}

	return true;
}

bool UActionPracticeGameplayAbility::CheckStaminaCost(const FGameplayAbilityActorInfo& ActorInfo) const
{
	if (StaminaCost <= 0.0f)
	{
		return true;
	}
	
	if (UActionPracticeAttributeSet* AttributeSet = GetActionPracticeAttributeSetFromActorInfo(&ActorInfo))
	{
		return AttributeSet->GetStamina() >= StaminaCost;
	}

	return false;
}

bool UActionPracticeGameplayAbility::ConsumeStamina()
{
	if (StaminaCost <= 0.0f)
	{
		return true;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	UActionPracticeAttributeSet* AttributeSet = GetActionPracticeAttributeSetFromActorInfo();

	if (ASC && AttributeSet)
	{
		// 스태미나 소모 GameplayEffect를 적용
		// 여기서는 직접 속성을 수정하는 방식을 사용
		const float CurrentStamina = AttributeSet->GetStamina();
		const float NewStamina = FMath::Max(0.0f, CurrentStamina - StaminaCost);
		
		// 속성 직접 설정 (실제 프로젝트에서는 GameplayEffect 사용 권장)
		const_cast<UActionPracticeAttributeSet*>(AttributeSet)->SetStamina(NewStamina);
		
		return true;
	}

	return false;
}

void UActionPracticeGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 스태미나 소모
	if (!ConsumeStamina())
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

AActionPracticeCharacter* UActionPracticeGameplayAbility::GetActionPracticeCharacterFromActorInfo() const
{
	return Cast<AActionPracticeCharacter>(GetActorInfo().AvatarActor.Get());
}

class AActionPracticeCharacter* UActionPracticeGameplayAbility::GetActionPracticeCharacterFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const
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

class UActionPracticeAttributeSet* UActionPracticeGameplayAbility::GetActionPracticeAttributeSetFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo(ActorInfo);
	if (Character)
	{
		return Character->GetAttributeSet();
	}
	return nullptr;
}

class UInputBufferComponent* UActionPracticeGameplayAbility::GetInputBufferComponentFromActorInfo() const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (Character)
	{
		return Character->GetInputBufferComponent();
	}
	return nullptr;
}
