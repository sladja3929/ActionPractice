#include "GAS/Abilities/JumpAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UJumpAbility::UJumpAbility()
{
	// GameplayTag는 Blueprint에서 설정
	
	// 스태미나 비용
	StaminaCost = 10.0f;
	
	// 기본값 설정
	JumpZVelocity = 600.0f;
	bAllowDoubleJump = false;
	DoubleJumpMultiplier = 0.8f;
	MaxJumpCount = 1;

	// 즉시 실행되는 어빌리티
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
}

bool UJumpAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 캐릭터가 점프 가능한 상태인지 확인
	if (const AActionPracticeCharacter* Character = Cast<AActionPracticeCharacter>(ActorInfo->AvatarActor.Get()))
	{
		const UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
		if (!MovementComp)
		{
			return false;
		}

		// 땅에 있거나 더블 점프 가능한 경우
		if (!MovementComp->IsFalling())
		{
			return true; // 땅에서 첫 번째 점프
		}
		else if (bAllowDoubleJump && CurrentJumpCount < MaxJumpCount)
		{
			return true; // 공중에서 더블 점프
		}
	}

	return false;
}

void UJumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	// 점프 실행
	if (CanJump())
	{
		PerformJump();
	}
	else if (CanDoubleJump())
	{
		PerformDoubleJump();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 착지 확인 타이머 시작
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			LandingCheckTimer,
			this,
			&UJumpAbility::CheckForLanding,
			0.1f,
			true
		);
	}

	// 점프는 즉시 완료되므로 바로 종료
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UJumpAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 타이머 정리는 착지 시에 처리

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UJumpAbility::PerformJump()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// 점프 속도 설정
	MovementComp->JumpZVelocity = JumpZVelocity;

	// 점프 실행
	Character->Jump();

	// 점프 횟수 증가
	CurrentJumpCount++;
	JumpStartTime = GetWorld()->GetTimeSeconds();

	UE_LOG(LogTemp, Warning, TEXT("Jump performed - Count: %d, Velocity: %f"), CurrentJumpCount, JumpZVelocity);
}

void UJumpAbility::PerformDoubleJump()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// 더블 점프 속도 계산
	float DoubleJumpVelocity = JumpZVelocity * DoubleJumpMultiplier;

	// 현재 수직 속도를 리셋하고 더블 점프 적용
	FVector CurrentVelocity = MovementComp->Velocity;
	CurrentVelocity.Z = DoubleJumpVelocity;
	MovementComp->Velocity = CurrentVelocity;

	// 점프 횟수 증가
	CurrentJumpCount++;
	JumpStartTime = GetWorld()->GetTimeSeconds();

	UE_LOG(LogTemp, Warning, TEXT("Double jump performed - Count: %d, Velocity: %f"), CurrentJumpCount, DoubleJumpVelocity);
}

bool UJumpAbility::CanJump() const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return false;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return false;
	}

	// 땅에 있고 첫 번째 점프인 경우
	return !MovementComp->IsFalling() && CurrentJumpCount == 0;
}

bool UJumpAbility::CanDoubleJump() const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character || !bAllowDoubleJump)
	{
		return false;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return false;
	}

	// 공중에 있고 더블 점프 가능한 경우
	return MovementComp->IsFalling() && CurrentJumpCount > 0 && CurrentJumpCount < MaxJumpCount;
}

void UJumpAbility::OnLanded()
{
	// 점프 횟수 리셋
	CurrentJumpCount = 0;

	// 착지 확인 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(LandingCheckTimer);
	}

	UE_LOG(LogTemp, Warning, TEXT("Landed - Jump count reset"));
}

void UJumpAbility::CheckForLanding()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// 착지했는지 확인
	if (!MovementComp->IsFalling() && CurrentJumpCount > 0)
	{
		OnLanded();
	}
}