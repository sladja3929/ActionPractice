#include "GAS/Abilities/Enemy/EnemyAttackAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/BossCharacter.h"
#include "GAS/AbilitySystemComponent/BossAbilitySystemComponent.h"
#include "GAS/Abilities/Tasks/AbilityTask_PlayMontageWithEvents.h"
#include "Items/AttackData.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogEnemyAttackAbility, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogEnemyAttackAbility, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

void UEnemyAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	PlayAction();
}

void UEnemyAttackAbility::SetHitDetectionConfig()
{
	//캐릭터에 공격 정보 제공
	ABossCharacter* BossCharacter = GetBossCharacterFromActorInfo();
	if (!BossCharacter)
	{
		DEBUG_LOG(TEXT("No Character"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	FGameplayTagContainer AssetTag = GetAssetTags();
	if (AssetTag.IsEmpty())
	{
		DEBUG_LOG(TEXT("No AssetTags"));
		return;
	}

	//HitDetectionSetter 초기화
	if (!HitDetectionSetter.Init(BossCharacter->GetHitDetectionInterface()))
	{
		DEBUG_LOG(TEXT("Failed to init HitDetectionSetter"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	//HitDetectionSetter 바인딩
	if (!HitDetectionSetter.Bind(this))
	{
		DEBUG_LOG(TEXT("Failed to bind HitDetectionSetter"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	//PrepareHitDetection 호출 (ComboCounter는 0으로 고정)
	if (!HitDetectionSetter.PrepareHitDetection(AssetTag, 0))
	{
		DEBUG_LOG(TEXT("Failed to prepare HitDetection"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	DEBUG_LOG(TEXT("Enemy Attack Ability: Call Hit Detection Prepare"));
}

void UEnemyAttackAbility::OnHitDetected(AActor* HitActor, const FHitResult& HitResult, FFinalAttackData AttackData)
{
	//Source ASC (공격자, AttackAbility 소유자)
	UBossAbilitySystemComponent* SourceASC = GetBossAbilitySystemComponentFromActorInfo();
	if (!HitActor || !SourceASC) return;

	//Target ASC (피격자)
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!TargetASC) return;

	//Source ASC에서 GE Spec 생성
	FGameplayEffectSpecHandle SpecHandle = SourceASC->CreateAttackGameplayEffectSpec(DamageInstantEffect, GetAbilityLevel(), this, AttackData);

	if (SpecHandle.IsValid())
	{
		//Target에게 적용
		FActiveGameplayEffectHandle ActiveGEHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

		//적용에 성공했으면
		if (ActiveGEHandle.WasSuccessfullyApplied())
		{
			DEBUG_LOG(TEXT("Damage Applied Successfully"));
		}
	}
}

void UEnemyAttackAbility::PlayAction()
{
	SetHitDetectionConfig();

	ExecuteMontageTask();
}

UAnimMontage* UEnemyAttackAbility::SetMontageToPlayTask()
{
	DEBUG_LOG(TEXT("SetMontageToPlayTask called"));
	return Montage;
}

void UEnemyAttackAbility::ExecuteMontageTask()
{
	UAnimMontage* MontageToPlay = SetMontageToPlayTask();
	if (!MontageToPlay)
	{
		DEBUG_LOG(TEXT("No Montage to Play"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// 커스텀 태스크 생성
	PlayMontageWithEventsTask = UAbilityTask_PlayMontageWithEvents::CreatePlayMontageWithEventsProxy(
		this,
		NAME_None,
		MontageToPlay,
		1.0f,
		NAME_None,
		1.0f
	);

	BindEventsAndReadyMontageTask();
}

void UEnemyAttackAbility::BindEventsAndReadyMontageTask()
{
	if (!PlayMontageWithEventsTask)
	{
		DEBUG_LOG(TEXT("No MontageWithEvents Task"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	//커스텀 몽타주 태스크 델리게이트 바인딩
	PlayMontageWithEventsTask->OnMontageCompleted.AddDynamic(this, &UEnemyAttackAbility::OnTaskMontageCompleted);
	PlayMontageWithEventsTask->OnMontageInterrupted.AddDynamic(this, &UEnemyAttackAbility::OnTaskMontageInterrupted);

	//태스크 활성화
	PlayMontageWithEventsTask->ReadyForActivation();
}

void UEnemyAttackAbility::OnTaskMontageCompleted()
{
	DEBUG_LOG(TEXT("Montage Task Completed"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UEnemyAttackAbility::OnTaskMontageInterrupted()
{
	DEBUG_LOG(TEXT("Montage Task Interrupted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UEnemyAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	DEBUG_LOG(TEXT("EndAbility %d"), bWasCancelled);

	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		//HitDetectionSetter 언바인딩
		HitDetectionSetter.UnBind();

		if (PlayMontageWithEventsTask)
		{
			PlayMontageWithEventsTask->bStopMontageWhenAbilityCancelled = bWasCancelled;
		}

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}