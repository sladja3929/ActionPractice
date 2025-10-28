#include "GAS/AbilitySystemComponent/ActionPracticeAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/AttributeSet/ActionPracticeAttributeSet.h"
#include "GAS/GameplayTagsSubsystem.h"
#include "Items/Weapon.h"
#include "Items/WeaponDataAsset.h"
#include "Items/AttackData.h"

#define ENABLE_DEBUG_LOG 0

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

void UActionPracticeAbilitySystemComponent::CalculateAndSetAttributes(AActor* SourceActor, const FFinalAttackData& FinalAttackData)
{
	bBlockedLastAttack = false;

	if (!CachedAPCharacter.IsValid())
	{
		Super::CalculateAndSetAttributes(SourceActor, FinalAttackData);
		return;
	}

	UActionPracticeAttributeSet* APAttributeSet = const_cast<UActionPracticeAttributeSet*>(GetActionPracticeAttributeSet());
	if (!APAttributeSet)
	{
		Super::CalculateAndSetAttributes(SourceActor, FinalAttackData);
		return;
	}

	//방어 태그 확인
	const FGameplayTag StateAbilityBlockingTag = UGameplayTagsSubsystem::GetStateAbilityBlockingTag();
	const bool bIsBlocking = HasMatchingGameplayTag(StateAbilityBlockingTag);

	if (bIsBlocking && SourceActor && CachedAPCharacter->GetLeftWeapon())
	{
		AWeapon* LeftWeapon = CachedAPCharacter->GetLeftWeapon();

		//공격자 방향 계산
		const FVector ToSource = SourceActor->GetActorLocation() - CachedAPCharacter->GetActorLocation();
		const FVector ToSourceNormalized = ToSource.GetSafeNormal2D();
		const FVector Forward = CachedAPCharacter->GetActorForwardVector();

		//캐릭터 정면과 공격 방향 사이의 각도 계산
		const float DotProduct = FVector::DotProduct(Forward, ToSourceNormalized);
		const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

		//블로킹 각도 범위
		const float BlockingAngle = 90.0f;

		if (AngleDegrees <= BlockingAngle)
		{
			//블로킹 성공
			bBlockedLastAttack = true;

			//무기의 DamageReduction 적용
			const FBlockActionData* BlockData = LeftWeapon->GetWeaponBlockData();
			const float DamageReduction = BlockData ? BlockData->DamageReduction : 0.0f;
			const float FinalDamage = FinalAttackData.FinalDamage * (1.0f - DamageReduction / 100.0f);

			//HP 적용
			const float OldHealth = APAttributeSet->GetHealth();
			APAttributeSet->SetHealth(FMath::Clamp(OldHealth - FinalDamage, 0.0f, APAttributeSet->GetMaxHealth()));

			//포이즈 대미지는 블로킹 시 감소
			if (FinalAttackData.PoiseDamage > 0.0f)
			{
				const float ReducedPoiseDamage = FinalAttackData.PoiseDamage * (1.0f - DamageReduction / 100.0f);
				const float OldPoise = APAttributeSet->GetPoise();
				APAttributeSet->SetPoise(FMath::Clamp(OldPoise - ReducedPoiseDamage, 0.0f, APAttributeSet->GetMaxPoise()));
			}

			DEBUG_LOG(TEXT("Blocked: Damage=%.1f, FinalDamage=%.1f, DamageReduction=%.1f%%, Health=%.1f/%.1f"),
				FinalAttackData.FinalDamage, FinalDamage, DamageReduction,
				APAttributeSet->GetHealth(), APAttributeSet->GetMaxHealth());
			return;
		}
	}

	//기본 or 방어실패: 기본 피격 계산식 사용
	Super::CalculateAndSetAttributes(SourceActor, FinalAttackData);
}

void UActionPracticeAbilitySystemComponent::HandleOnDamagedResolved(AActor* SourceActor, const FFinalAttackData& FinalAttackData)
{
	if (bBlockedLastAttack)
	{
		DEBUG_LOG(TEXT("Block success - TODO: Trigger BlockAbility event"));
		//TODO: BlockAbility에 이벤트 트리거
		return;
	}

	//기본 or 방어실패: 기본 피격 로직
	Super::HandleOnDamagedResolved(SourceActor, FinalAttackData);
}