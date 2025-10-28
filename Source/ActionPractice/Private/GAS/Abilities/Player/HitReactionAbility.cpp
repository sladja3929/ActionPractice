#include "GAS/Abilities/Player/HitReactionAbility.h"
#include "Animation/AnimMontage.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogHitReactionAbility, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogHitReactionAbility, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

UHitReactionAbility::UHitReactionAbility()
{
	bRotateBeforeAction = false;
	StaminaCost = -1.0f; // 스태미나 체크 안함
}

void UHitReactionAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (TriggerEventData)
	{
		const float PoiseValue = TriggerEventData->EventMagnitude;
		DEBUG_LOG(TEXT("HitReaction activated with Poise=%.1f"), PoiseValue);

		//Poise 값에 따라 다른 몽타주 선택 (일단 기본 몽타주만 사용)
		if (PoiseValue < -50.0f)
		{
			DEBUG_LOG(TEXT("Heavy hit reaction (Poise < -50)"));
			//TODO: HeavyHitReactionMontage 사용
		}
		else if (PoiseValue < -20.0f)
		{
			DEBUG_LOG(TEXT("Medium hit reaction (-50 <= Poise < -20)"));
			//TODO: MediumHitReactionMontage 사용
		}
		else
		{
			DEBUG_LOG(TEXT("Light hit reaction (Poise >= -20)"));
			//TODO: LightHitReactionMontage 사용
		}
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

UAnimMontage* UHitReactionAbility::SetMontageToPlayTask()
{
	return HitReactionMontage;
}