#include "GAS/Abilities/SprintAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
	#define DEBUG_LOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)
#else
	#define DEBUG_LOG(Format, ...)
#endif

USprintAbility::USprintAbility()
{
	// GameplayTag는 Blueprint에서 설정
	
	// 초기 스태미나 비용 없음 (지속적으로 소모)
	StaminaCost = 0.0f;
	
	// 기본값 설정
	SprintSpeedMultiplier = 1.5f;
	StaminaDrainPerSecond = 12.0f;
	MinStaminaToStart = 10.0f;
	MinStaminaToContinue = 5.0f;

	// 인스턴싱 정책: 지속적인 어빌리티이므로 액터별 인스턴스
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool USprintAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 최소 스태미나 확인
	if (const UActionPracticeAttributeSet* AttributeSet = Cast<UActionPracticeAttributeSet>(ActorInfo->AbilitySystemComponent->GetAttributeSet(UActionPracticeAttributeSet::StaticClass())))
	{
		return AttributeSet->GetStamina() >= MinStaminaToStart;
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
	// 스프린트 종료
	StopSprinting();
	DEBUG_LOG(TEXT("sprint end"));
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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

	// 원래 이동 속도 저장
	OriginalMaxWalkSpeed = MovementComp->MaxWalkSpeed;

	// 스프린트 속도 적용
	MovementComp->MaxWalkSpeed = OriginalMaxWalkSpeed * SprintSpeedMultiplier;

	// 스태미나 소모 타이머 시작
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			StaminaDrainTimer,
			this,
			&USprintAbility::DrainStamina,
			1.0f / StaminaDrainPerSecond,
			true
		);

		// 스프린트 조건 확인 타이머 시작 (더 자주 체크)
		GetWorld()->GetTimerManager().SetTimer(
			SprintCheckTimer,
			this,
			&USprintAbility::CheckSprintConditions,
			0.1f,
			true
		);
	}

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
			// 원래 이동 속도 복구
			MovementComp->MaxWalkSpeed = OriginalMaxWalkSpeed;
		}
	}

	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaDrainTimer);
		GetWorld()->GetTimerManager().ClearTimer(SprintCheckTimer);
	}

	DEBUG_LOG(TEXT("Sprint ended"));
}

void USprintAbility::HandleSprinting()
{
	// 스프린트 중 추가 처리가 필요하면 여기에 구현
}

bool USprintAbility::CanContinueSprinting() const
{
	UActionPracticeAttributeSet* AttributeSet = GetActionPracticeAttributeSetFromActorInfo();
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();

	if (!AttributeSet || !Character)
	{
		return false;
	}

	// 스태미나 확인
	if (AttributeSet->GetStamina() < MinStaminaToContinue)
	{
		DEBUG_LOG(TEXT("CanContinueSprinting Stop - No Stamina"));
		return false;
	}

	// 이동 입력 확인 (실제로는 Enhanced Input에서 확인해야 함)
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp && MovementComp->GetCurrentAcceleration().Size() < 10.0f)
	{
		return false; // 이동 입력이 없으면 스프린트 중단
	}

	// 공중에 있으면 스프린트 불가
	if (MovementComp && MovementComp->IsFalling())
	{
		DEBUG_LOG(TEXT("CanContinueSprinting Stop - Is Falling"));
		return false;
	}

	return true;
}

void USprintAbility::DrainStamina()
{
	UActionPracticeAttributeSet* AttributeSet = GetActionPracticeAttributeSetFromActorInfo();
	if (AttributeSet)
	{
		float CurrentStamina = AttributeSet->GetStamina();
		float NewStamina = FMath::Max(0.0f, CurrentStamina - 1.0f);
		const_cast<UActionPracticeAttributeSet*>(AttributeSet)->SetStamina(NewStamina);
	}
}

void USprintAbility::CheckSprintConditions()
{
	if (!CanContinueSprinting())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}