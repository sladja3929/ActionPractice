#include "GAS/ActionPracticeAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/ActionPracticeAbilitySystemComponent.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogPlayerStatsWidget, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogPlayerStatsWidget, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

UActionPracticeAttributeSet::UActionPracticeAttributeSet()
{
	InitStrength(10.0f);
	InitDexterity(10.0f);
	InitPhysicalAttackPower(50.0f);
}

void UActionPracticeAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, Dexterity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, PhysicalAttackPower, COND_None, REPNOTIFY_Always);
}

void UActionPracticeAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetStrengthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); // 최소 1
	}
	else if (Attribute == GetDexterityAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); // 최소 1
	}
}

void UActionPracticeAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	if (Attribute == GetStrengthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); // 최소 1
	}
	else if (Attribute == GetDexterityAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); // 최소 1
	}
}

void UActionPracticeAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	//Secondary Attribute를 변경하는 것들 처리
	if (Data.EvaluatedData.Attribute == GetStrengthAttribute() ||
		Data.EvaluatedData.Attribute == GetDexterityAttribute())
	{
		CalculateSecondaryAttributes();
	}
}

void UActionPracticeAttributeSet::CalculateSecondaryAttributes()
{
	// Calculate Physical Attack Power based on Strength and Dexterity
	// 기본 공격력 + 스텟 보너스
	const float BaseAttackPower = 50.0f;
	const float StrengthBonus = GetStrength() * 0.8f;
	const float DexterityBonus = GetDexterity() * 0.6f;
	
	SetPhysicalAttackPower(BaseAttackPower + StrengthBonus + DexterityBonus);
}

float UActionPracticeAttributeSet::CalculateWeaponDamageBonus(float StrengthScaling, float DexterityScaling) const
{
	// 엘든링 스타일 무기 스케일링 계산
	const float StrengthBonus = GetStrength() * StrengthScaling * 0.01f; // 스케일링을 퍼센트로 계산
	const float DexterityBonus = GetDexterity() * DexterityScaling * 0.01f;
	
	return StrengthBonus + DexterityBonus;
}

// Rep Notify Functions
void UActionPracticeAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, Strength, OldStrength);
}

void UActionPracticeAttributeSet::OnRep_Dexterity(const FGameplayAttributeData& OldDexterity)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, Dexterity, OldDexterity);
}

void UActionPracticeAttributeSet::OnRep_PhysicalAttackPower(const FGameplayAttributeData& OldPhysicalAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, PhysicalAttackPower, OldPhysicalAttackPower);
}