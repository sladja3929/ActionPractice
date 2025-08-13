#include "GAS/Abilities/RollAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

URollAbility::URollAbility()
{
	// GameplayTag는 Blueprint에서 설정
	
	// 스태미나 비용
	StaminaCost = 20.0f;
	
	// 기본값 설정
	RollDistance = 400.0f;
	RollSpeed = 800.0f;
	InvincibilityFrames = 0.5f;
	RecoveryTime = 0.3f;
}

void URollAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	// 구르기 실행
	PerformRoll();
}

void URollAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 무적 상태 종료
	EndInvincibility();

	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(InvincibilityTimer);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void URollAbility::PerformRoll()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character || !RollMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 구르기 방향 계산
	FVector RollDirection = CalculateRollDirection();
	
	// 캐릭터 회전 설정
	if (RollDirection.Size() > 0.1f)
	{
		Character->SetActorRotation(RollDirection.Rotation());
	}

	// 몽타주 재생
	if (!MontageEndedDelegate.IsBound())
	{
		MontageEndedDelegate = FOnMontageEnded::CreateUObject(this, &URollAbility::OnMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, RollMontage);
	}

	AnimInstance->Montage_Play(RollMontage);

	// 구르기 시작 시간 기록
	RollStartTime = GetWorld()->GetTimeSeconds();

	// 무적 상태 시작
	StartInvincibility();

	// 이동 임펄스 적용 (더 자연스러운 구르기 위해)
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp)
	{
		// 중력 일시 감소 (구르기 느낌)
		float OriginalGravity = MovementComp->GravityScale;
		MovementComp->GravityScale = 0.5f;
		
		// 구르기 후 중력 복구
		FTimerHandle GravityTimer;
		GetWorld()->GetTimerManager().SetTimer(
			GravityTimer,
			[MovementComp, OriginalGravity]()
			{
				if (MovementComp)
				{
					MovementComp->GravityScale = OriginalGravity;
				}
			},
			0.8f,
			false
		);

		// 구르기 방향으로 임펄스 적용
		FVector LaunchVelocity = RollDirection * RollSpeed;
		LaunchVelocity.Z = 0.0f; // 수평 이동만
		Character->LaunchCharacter(LaunchVelocity, true, true);
	}

	UE_LOG(LogTemp, Warning, TEXT("Roll performed in direction: %s"), *RollDirection.ToString());
}

FVector URollAbility::CalculateRollDirection() const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return FVector::ForwardVector;
	}

	// 입력 방향 가져오기 (현재 캐릭터의 MovementInputVector 사용)
	// 실제로는 Enhanced Input에서 직접 가져와야 하지만, 여기서는 캐릭터의 현재 이동 방향 사용
	FVector InputDirection = Character->GetVelocity();
	InputDirection.Z = 0.0f;
	InputDirection.Normalize();

	// 입력이 없으면 캐릭터가 바라보는 방향으로
	if (InputDirection.Size() < 0.1f)
	{
		InputDirection = Character->GetActorForwardVector();
	}

	return InputDirection;
}

void URollAbility::StartInvincibility()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	// 무적 태그 추가
	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (ASC)
	{
		FGameplayTagContainer InvincibilityTags;
		// 런타임에 태그 생성 (안전하지 않을 수 있음)
		FGameplayTag InvincibleTag = FGameplayTag::RequestGameplayTag(FName("State.Invincible"), false);
		if (InvincibleTag.IsValid())
		{
			InvincibilityTags.AddTag(InvincibleTag);
			ASC->AddLooseGameplayTags(InvincibilityTags);
		}
	}

	// 무적 상태 타이머 설정
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			InvincibilityTimer,
			this,
			&URollAbility::EndInvincibility,
			InvincibilityFrames,
			false
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("Invincibility started for %f seconds"), InvincibilityFrames);
}

void URollAbility::EndInvincibility()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	// 무적 태그 제거
	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (ASC)
	{
		FGameplayTagContainer InvincibilityTags;
		// 런타임에 태그 생성 (안전하지 않을 수 있음)
		FGameplayTag InvincibleTag = FGameplayTag::RequestGameplayTag(FName("State.Invincible"), false);
		if (InvincibleTag.IsValid())
		{
			InvincibilityTags.AddTag(InvincibleTag);
			ASC->RemoveLooseGameplayTags(InvincibilityTags);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Invincibility ended"));
}

void URollAbility::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == RollMontage)
	{
		// 회복 시간 후 어빌리티 종료
		if (GetWorld())
		{
			FTimerHandle RecoveryTimer;
			GetWorld()->GetTimerManager().SetTimer(
				RecoveryTimer,
				[this]()
				{
					EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
				},
				RecoveryTime,
				false
			);
		}
		else
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, bInterrupted);
		}
	}
}