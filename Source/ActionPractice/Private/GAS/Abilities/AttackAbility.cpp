#include "GAS/Abilities/AttackAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Items/Weapon.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"

UAttackAbility::UAttackAbility()
{
	// GameplayTag는 Blueprint에서 설정
	
	StaminaCost = 15.0f;
	ComboCounter = 0;
	MaxComboCount = 3;
}

void UAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("Attack Ability Activated"));
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 무기 정보 가져오기
	if (AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo())
	{
		if (AWeapon* Weapon = Character->GetRightWeapon())
		{
			AttackData = Weapon->GetWeaponAttackDataByTag(FGameplayTag::RequestGameplayTag("Ability.Attack"));
			AttackMontage = AttackData->AttackMontage;
			MaxComboCount = AttackData->ComboAttackData.Num();
		}

		else EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
	
	// ===== 이벤트 리스닝 시작 =====
	UAbilityTask_WaitGameplayEvent* WaitEnableComboInput = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag_EnableComboInput);
	WaitEnableComboInput->EventReceived.AddDynamic(this, &UAttackAbility::HandleEnableComboInputEvent);
	WaitEnableComboInput->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitRecoveryEnd = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag_AttackRecoveryEnd);
	WaitRecoveryEnd->EventReceived.AddDynamic(this, &UAttackAbility::HandleAttackRecoveryEndEvent);
	WaitRecoveryEnd->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* WaitResetCombo = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag_ResetCombo);
	WaitResetCombo->EventReceived.AddDynamic(this, &UAttackAbility::HandleResetComboEvent);
	WaitResetCombo->ReadyForActivation();
	
	PerformAttack();
}

void UAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bWasCancelled)
	{
		AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
		if (Character && AttackMontage)
		{
			Character->StopAnimMontage(AttackMontage.Get());
		}
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Attacking"));
	}
	
	ResetCombo();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UAttackAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	PerformAttack();
}

/* 몽타주에 노티파이 넣는 순서 (수행 매커니즘)
 * 1. 몽타주 실행
 *		bIsAttacking = true
 * 2. enablecomboInput = 입력으로 다음공격, 구르기 저장가능 지점 (구르기 저장중이어도 다음공격 우선)
 *		bCanComboSave = true (bCanRollSave = false로) / bCanRollSave = true (bCanComboSave = true면 X)
 * 3. AttackRecoveryEnd / CheckComboInput = 공격 선딜이 끝나는 지점, 저장했으면 자동으로 다음공격 수행 (끝나고 이동, 구르기 가능)
 *		bIsAttacking = false
 * 4. ResetCombo = 공격 콤보 끝남 (2-3 사이 공격하면 다음 콤보)
 *
 */

void UAttackAbility::PerformAttack()
{
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!Character || !AttackMontage || !AnimInstance)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	
	if (!bIsAttacking) //공격 가능 구간일때
	{
		if (!bCanComboSave) //첫 공격
		{
			ComboCounter = 0;
			TryAttack();
		}

		else if (ComboCounter < MaxComboCount) //선딜 이후 공격
		{
			TryAttack();
		}
	}
	
	else //공격 선딜일때 저장
	{
		SaveComboInput();
	}
}

void UAttackAbility::TryAttack() //실제 공격이 이루어지는 함수
{
	if (!ConsumeStamina())
	{
		//endability말고 기다리는걸로 바꿔야함
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Attacking"));
	}
	
	bIsAttacking = true;
	bComboInputSaved = false;
	ComboCounter++;

	PlayAttackMontage();
	//실제 데미지 판정 함수
}


void UAttackAbility::SaveComboInput() 
{
	if (bCanComboSave && ComboCounter < MaxComboCount && !bComboInputSaved)
	{
		bComboInputSaved = true;
	}
}

void UAttackAbility::PlayAttackMontage()
{	
	AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
	if (!Character) return;

	FName SectionName = FName(*FString::Printf(TEXT("Attack%d"), ComboCounter));
	
	if (ComboCounter == 1)
	{
		Character->PlayAnimMontage(AttackMontage.Get(), 1.0f, SectionName);
	}

	else if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage.Get());
	}
		
	//UE_LOG(LogTemp, Warning, TEXT("Combo Continued: %s"), *SectionName.ToString());
}

void UAttackAbility::CheckComboInput()
{
	if (bComboInputSaved && ComboCounter < MaxComboCount)
	{
		TryAttack();
	}
}

void UAttackAbility::EnableComboInput()
{
	bCanComboSave = true;
}

void UAttackAbility::AttackRecoveryEnd()
{
	bIsAttacking = false;
	bCanComboSave = true;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Attacking"));
	}
}

void UAttackAbility::ResetCombo()
{
	ComboCounter = 0;
	bComboInputSaved = false;
	bCanComboSave = false;
	
	UE_LOG(LogTemp, Warning, TEXT("Combo Reset"));
}

void UAttackAbility::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == AttackMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UAttackAbility::HandleEnableComboInputEvent(FGameplayEventData Payload)
{
	EnableComboInput();
}

void UAttackAbility::HandleAttackRecoveryEndEvent(FGameplayEventData Payload)
{
	//이동과 콤보입력중 우선순위 정하기
	AttackRecoveryEnd();
	CheckComboInput();
}

void UAttackAbility::HandleResetComboEvent(FGameplayEventData Payload)
{
	ResetCombo();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}