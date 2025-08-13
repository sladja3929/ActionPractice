#include "GAS/Abilities/BlockAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Items/Weapon.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UBlockAbility::UBlockAbility()
{
	// GameplayTag는 Blueprint에서 설정
	
	// 초기 스태미나 비용 (방어 시작 시)
	StaminaCost = 5.0f;
	
	// 기본값 설정
	StaminaDrainPerSecond = 8.0f;
	DamageReductionMultiplier = 0.2f; // 80% 데미지 감소
	StaminaDamageReduction = 0.5f;
	MovementSpeedMultiplier = 0.3f;
	BlockAngle = 120.0f;
	ParryWindow = 0.3f;
	GuardBreakThreshold = 50.0f;
}

void UBlockAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 방어 시작
	StartBlocking();
}

void UBlockAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 방어 종료
	StopBlocking();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlockAbility::StartBlocking()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	bIsBlocking = true;
	BlockStartTime = GetWorld()->GetTimeSeconds();

	// 초기 스태미나 소모
	ConsumeStamina();

	// 이동 속도 감소
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp)
	{
		float BaseSpeed = MovementComp->MaxWalkSpeed / MovementSpeedMultiplier;
		MovementComp->MaxWalkSpeed = BaseSpeed * MovementSpeedMultiplier;
	}

	// 방어 시작 몽타주 재생
	if (BlockStartMontage)
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			// 몽타주 종료 델리게이트 바인딩
			if (!MontageEndedDelegate.IsBound())
			{
				MontageEndedDelegate = FOnMontageEnded::CreateUObject(this, &UBlockAbility::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, BlockStartMontage);
			}
			
			AnimInstance->Montage_Play(BlockStartMontage);
		}
	}

	// 스태미나 지속 소모 타이머 시작
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			StaminaDrainTimer,
			this,
			&UBlockAbility::DrainStamina,
			1.0f / StaminaDrainPerSecond,
			true
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("Block started"));
}

void UBlockAbility::HandleBlocking(float DeltaTime)
{
	// 지속적인 방어 처리 (필요시 구현)
}

void UBlockAbility::StopBlocking()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	bIsBlocking = false;

	// 스태미나 소모 타이머 정지
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaDrainTimer);
	}

	// 이동 속도 복구
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (MovementComp)
	{
		MovementComp->MaxWalkSpeed = MovementComp->MaxWalkSpeed / MovementSpeedMultiplier;
	}

	// 방어 종료 몽타주 재생
	if (BlockEndMontage)
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(BlockEndMontage);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Block ended"));
}

void UBlockAbility::HandleBlockSuccess(float IncomingDamage, AActor* DamageSource)
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	UActionPracticeAttributeSet* AttributeSet = GetActionPracticeAttributeSetFromActorInfo();
	
	if (!Character || !AttributeSet)
	{
		return;
	}

	// 패리 윈도우 체크
	if (IsInParryWindow())
	{
		HandleParrySuccess(DamageSource);
		return;
	}

	// 방어 효율성 계산
	float BlockEffectiveness = CalculateBlockEffectiveness();
	
	// 감소된 데미지 계산
	float ReducedDamage = IncomingDamage * DamageReductionMultiplier * (1.0f - BlockEffectiveness);
	
	// 스태미나 데미지 계산
	float StaminaDamage = IncomingDamage * StaminaDamageReduction;
	
	// 방어구 및 무기 고려
	AWeapon* Shield = Character->GetLeftWeapon();
	if (Shield && Shield->GetWeaponType() == WeaponEnums::Shield)
	{
		//ReducedDamage *= (1.0f - Shield->GetPhysicalReduction() / 100.0f);
		//StaminaDamage *= (1.0f - Shield->GetStaminaDamage() / 100.0f);
	}

	// 스태미나 소모
	float CurrentStamina = AttributeSet->GetStamina();
	float NewStamina = FMath::Max(0.0f, CurrentStamina - StaminaDamage);
	const_cast<UActionPracticeAttributeSet*>(AttributeSet)->SetStamina(NewStamina);

	// 가드 브레이크 체크
	if (StaminaDamage >= GuardBreakThreshold || NewStamina <= 0.0f)
	{
		HandleGuardBreak();
		return;
	}

	// 남은 데미지 적용
	if (ReducedDamage > 0.0f)
	{
		float CurrentHealth = AttributeSet->GetHealth();
		float NewHealth = FMath::Max(0.0f, CurrentHealth - ReducedDamage);
		const_cast<UActionPracticeAttributeSet*>(AttributeSet)->SetHealth(NewHealth);
	}

	UE_LOG(LogTemp, Warning, TEXT("Blocked attack! Reduced damage: %f, Stamina damage: %f"), ReducedDamage, StaminaDamage);
}

void UBlockAbility::HandleParrySuccess(AActor* DamageSource)
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	// 패리 성공 몽타주 재생
	if (BlockSuccessMontage)
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(BlockSuccessMontage);
		}
	}

	// 패리 성공 시 상대방을 스턴시키거나 카운터 어택 기회 제공
	// TODO: 상대방에게 스턴 효과 적용

	UE_LOG(LogTemp, Warning, TEXT("Parry successful!"));
}

void UBlockAbility::HandleGuardBreak()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return;
	}

	// 가드 브레이크 몽타주 재생
	if (BlockFailMontage)
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(BlockFailMontage);
		}
	}

	// 방어 강제 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

	// TODO: 스턴 효과 적용

	UE_LOG(LogTemp, Warning, TEXT("Guard broken!"));
}

bool UBlockAbility::CanBlockFromDirection(const FVector& DamageDirection) const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character)
	{
		return false;
	}

	FVector CharacterForward = Character->GetActorForwardVector();
	FVector NormalizedDamageDir = DamageDirection.GetSafeNormal();

	float DotProduct = FVector::DotProduct(CharacterForward, -NormalizedDamageDir);
	float AngleInDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	return AngleInDegrees <= (BlockAngle * 0.5f);
}

float UBlockAbility::CalculateBlockEffectiveness() const
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	UActionPracticeAttributeSet* AttributeSet = GetActionPracticeAttributeSetFromActorInfo();
	
	if (!Character || !AttributeSet)
	{
		return 0.0f;
	}

	// 기본 방어 효율성
	float Effectiveness = 0.5f;

	// 방패가 있으면 효율성 증가
	AWeapon* Shield = Character->GetLeftWeapon();
	if (Shield && Shield->GetWeaponType() == WeaponEnums::Shield)
	{
		Effectiveness += 0.3f;
		
		// 방패의 방어 스탯 고려
		//Effectiveness += (Shield->GetPhysicalReduction() / 100.0f) * 0.2f;
	}

	// 근력에 따른 방어 효율성 (방패 사용 시)
	if (Shield)
	{
		float StrengthBonus = (AttributeSet->GetStrength() / 100.0f) * 0.2f;
		Effectiveness += StrengthBonus;
	}

	return FMath::Clamp(Effectiveness, 0.0f, 1.0f);
}

bool UBlockAbility::IsInParryWindow() const
{
	if (!bIsBlocking)
	{
		return false;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();
	return (CurrentTime - BlockStartTime) <= ParryWindow;
}

void UBlockAbility::DrainStamina()
{
	UActionPracticeAttributeSet* AttributeSet = GetActionPracticeAttributeSetFromActorInfo();
	if (AttributeSet)
	{
		float CurrentStamina = AttributeSet->GetStamina();
		float NewStamina = FMath::Max(0.0f, CurrentStamina - 1.0f);
		const_cast<UActionPracticeAttributeSet*>(AttributeSet)->SetStamina(NewStamina);

		// 스태미나가 부족하면 방어 종료
		if (NewStamina <= 0.0f)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		}
	}
}

void UBlockAbility::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == BlockStartMontage)
	{
		// 방어 유지 몽타주로 전환 (있는 경우)
		if (BlockIdleMontage && bIsBlocking)
		{
			AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
			if (Character)
			{
				UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
				if (AnimInstance)
				{
					AnimInstance->Montage_Play(BlockIdleMontage);
				}
			}
		}
	}
}