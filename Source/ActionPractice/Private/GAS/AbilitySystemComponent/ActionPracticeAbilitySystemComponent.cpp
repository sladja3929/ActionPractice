#include "GAS/AbilitySystemComponent/ActionPracticeAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/AttributeSet/ActionPracticeAttributeSet.h"
#include "GAS/GameplayTagsSubsystem.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogBaseAbilitySystemComponent, Log, All);
	#define DEBUG_LOG(Format, ...) UE_LOG(LogBaseAbilitySystemComponent, Warning, Format, ##__VA_ARGS__)
#else
	#define DEBUG_LOG(Format, ...)
#endif

UActionPracticeAbilitySystemComponent::UActionPracticeAbilitySystemComponent()
{
}

void UActionPracticeAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		CachedAPCharacter = Cast<AActionPracticeCharacter>(Owner);
	}

	EffectStaminaRegenBlockDurationTag = UGameplayTagsSubsystem::GetEffectStaminaRegenBlockDurationTag();
	if (!EffectStaminaRegenBlockDurationTag.IsValid())
	{
		DEBUG_LOG(TEXT("EffectStaminaRegenBlockDurationTag is Invalid"));
	}
}

void UActionPracticeAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	CachedAPCharacter = Cast<AActionPracticeCharacter>(InOwnerActor);
}

const UActionPracticeAttributeSet* UActionPracticeAbilitySystemComponent::GetActionPracticeAttributeSet() const 
{
	return this->GetSet<UActionPracticeAttributeSet>();
}

void UActionPracticeAbilitySystemComponent::ApplyStaminaRegenBlock(float Duration)
{
	DEBUG_LOG(TEXT("========= ApplyStaminaRegenBlock START ========="));
	DEBUG_LOG(TEXT("Requested Duration: %.3f seconds"), Duration);
	if (!StaminaRegenBlockEffect)
	{
		DEBUG_LOG(TEXT("StaminaRegenBlockEffect is not set"));
		return;
	}

	//기존 활성 GE가 있으면 남은 시간과 비교
	/*if (StaminaRegenBlockHandle.IsValid())
	{
		//현재 활성화된 해당 GE 가져옴
		const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
		const FActiveGameplayEffect* ActiveGE = GetActiveGameplayEffect(StaminaRegenBlockHandle);
		//const FActiveGameplayEffect* ActiveGE = ActiveGameplayEffects.GetActiveGameplayEffect(StaminaRegenBlockHandle);
		
		if (!ActiveGE)
		{
			DEBUG_LOG(TEXT("No Active GE_StaminaRegenBlock"));
			StaminaRegenBlockHandle = FActiveGameplayEffectHandle();
		}
		
		if (ActiveGE)
		{
			const float Remaining = ActiveGE->GetTimeRemaining(Now);
			//필요하면 전체 지속시간도 확인 가능
			//const float Total = ActiveGE->GetDuration();

			if (Duration <= Remaining)
			{
				DEBUG_LOG(TEXT("Skip reapply: New(%.2fs) <= Remaining(%.2fs)"), Duration, Remaining);
				return;
			}
		}
	}*/

	//Spec 생성
	FGameplayEffectContextHandle Context = MakeEffectContext();
	Context.AddSourceObject(this);

	const float Level = 1.0f;
	FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(StaminaRegenBlockEffect, Level, Context);
	if (!Spec.IsValid())
	{
		DEBUG_LOG(TEXT("Failed to create GE_StaminaRegenBlock spec"));
		return;
	}

	if (EffectStaminaRegenBlockDurationTag.IsValid())
	{
		Spec.Data.Get()->SetSetByCallerMagnitude(EffectStaminaRegenBlockDurationTag, Duration);
		Spec.Data.Get()->SetDuration(Duration, true);
	}

	//더 길게 갱신해야 하는 경우에만 제거 후 재적용
	if (StaminaRegenBlockHandle.IsValid())
	{
		RemoveActiveGameplayEffect(StaminaRegenBlockHandle);
		StaminaRegenBlockHandle = FActiveGameplayEffectHandle();
	}

	StaminaRegenBlockHandle = ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	//적용 후 검증
	if (StaminaRegenBlockHandle.IsValid())
	{
		const FActiveGameplayEffect* AppliedGE = GetActiveGameplayEffect(StaminaRegenBlockHandle);
		if (AppliedGE)
		{
			const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
			float AppliedDuration = AppliedGE->GetDuration();
			float TimeRemaining = AppliedGE->GetTimeRemaining(Now);
			float EndTime = AppliedGE->GetEndTime();
			
			DEBUG_LOG(TEXT("Applied GE Info:"));
			DEBUG_LOG(TEXT("  - Duration: %.3f"), AppliedDuration);
			DEBUG_LOG(TEXT("  - Time Remaining: %.3f"), TimeRemaining);
			DEBUG_LOG(TEXT("  - End Time: %.3f"), EndTime);
			DEBUG_LOG(TEXT("  - Current Time: %.3f"), Now);
			
			//Period 확인
			if (AppliedGE->Spec.GetPeriod() > 0)
			{
				DEBUG_LOG(TEXT("  - WARNING: Period is set to %.3f"), AppliedGE->Spec.GetPeriod());
			}
		}
	}
	else
	{
		DEBUG_LOG(TEXT("ERROR: Failed to apply GE"));
	}
	
	DEBUG_LOG(TEXT("========= ApplyStaminaRegenBlock END ========="));
	
	/*DEBUG_LOG(TEXT("Apply/Reapply StaminaRegenBlock: New=%.2fs, HandleValid=%s"),
		Duration,
		StaminaRegenBlockHandle.IsValid() ? TEXT("true") : TEXT("false"));*/
}