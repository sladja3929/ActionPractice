#include "GAS/Abilities/SprintAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogSprintAbility, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogSprintAbility, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

USprintAbility::USprintAbility()
{
	StaminaCost = 0.1f;
	SprintSpeedMultiplier = 1.5f;
}

bool USprintAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (const UActionPracticeAttributeSet* AttributeSet = Cast<UActionPracticeAttributeSet>(ActorInfo->AbilitySystemComponent->GetAttributeSet(UActionPracticeAttributeSet::StaticClass())))
	{
		return AttributeSet->GetStamina() > 0;
	}

	return false;
}

void USprintAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	SprintSpeedMultiplier = Character->SprintSpeedMultiplier;
	DEBUG_LOG(TEXT("Sprint Ability Activated"));
	StartSprinting();
}

void USprintAbility::StartSprinting()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!StartStaminaDrain())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	//스프린트 조건 확인 타이머
	if (GetWorld())
	{
		
		GetWorld()->GetTimerManager().SetTimer(
			SprintCheckTimer,
			this,
			&USprintAbility::CheckSprintConditions,
			0.1f,
			true
		);
	}

	OriginalMaxWalkSpeed = MovementComp->MaxWalkSpeed;
	MovementComp->MaxWalkSpeed = OriginalMaxWalkSpeed * SprintSpeedMultiplier;

	DEBUG_LOG(TEXT("Sprint started - Speed: %f"), MovementComp->MaxWalkSpeed);
}

void USprintAbility::StopSprinting()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (Character)
	{
		UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
		if (MovementComp && OriginalMaxWalkSpeed > 0.0f)
		{
			//원래 이동 속도 복구
			MovementComp->MaxWalkSpeed = OriginalMaxWalkSpeed;
		}
	}

	StopStaminaDrain();
	
	//타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(SprintCheckTimer);
	}

	DEBUG_LOG(TEXT("Sprint ended"));
}

void USprintAbility::HandleSprinting()
{
	// 스프린트 중 추가 처리가 필요할때
}

bool USprintAbility::CanContinueSprinting() const
{
	UActionPracticeAttributeSet* AttributeSet = GetActionPracticeAttributeSetFromActorInfo();
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();

	if (!AttributeSet || !Character)
	{
		return false;
	}

	//스테미나 부족
	if (AttributeSet->GetStamina() <= 0)
	{
		DEBUG_LOG(TEXT("CanContinueSprinting Stop - No Stamina"));
		return false;
	}

	//이동 입력이 없으면
	FVector2D MovementInput = Character->GetCurrentMovementInput();
	DEBUG_LOG(TEXT("Real-time MovementInput: %f"), MovementInput.Size());
	if (MovementInput.Size() < 0.1f)
	{
		DEBUG_LOG(TEXT("CanContinueSprinting Stop - No Movement Input"));
		return false;
	}

	//공중에 있으면
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp && MovementComp->IsFalling())
	{
		DEBUG_LOG(TEXT("CanContinueSprinting Stop - Is Falling"));
		return false;
	}

	return true;
}

bool USprintAbility::StartStaminaDrain()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	// 기존 드레인이 살아있으면 재설정(해제 후 재적용)
	if (StaminaDrainHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(StaminaDrainHandle);
		StaminaDrainHandle = FActiveGameplayEffectHandle();
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	const float EffectiveLevel = static_cast<float>(GetAbilityLevel());
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StaminaDrainEffect, EffectiveLevel, EffectContext);
	
	if (!SpecHandle.IsValid())
	{
		DEBUG_LOG(TEXT("failed StaminaDrain GameplayEffectSpec"));
		return false;
	}

	SpecHandle.Data.Get()->SetSetByCallerMagnitude(EffectStaminaCostTag, -StaminaCost);
	StaminaDrainHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	const bool bApplied = StaminaDrainHandle.IsValid();
	
	DEBUG_LOG(TEXT("StartStaminaDrain applied=%s, DrainPerPeriod=%.2f"), bApplied ? TEXT("true") : TEXT("false"), StaminaCost);
	
	return bApplied;
}

void USprintAbility::StopStaminaDrain()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		DEBUG_LOG(TEXT("No ASC"));
		StaminaDrainHandle = FActiveGameplayEffectHandle();
		return;
	}

	if (StaminaDrainHandle.IsValid())
	{
		const int32 Removed = ASC->RemoveActiveGameplayEffect(StaminaDrainHandle);
		StaminaDrainHandle = FActiveGameplayEffectHandle();
		DEBUG_LOG(TEXT("StopStaminaDrain removed=%d"), Removed);
	}
}

void USprintAbility::CheckSprintConditions()
{
	if (!CanContinueSprinting())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void USprintAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	DEBUG_LOG(TEXT("Sprint Input Released - End Ability"));
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void USprintAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	DEBUG_LOG(TEXT("sprint cancel"));
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void USprintAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	StopSprinting();
	DEBUG_LOG(TEXT("sprint end"));
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
