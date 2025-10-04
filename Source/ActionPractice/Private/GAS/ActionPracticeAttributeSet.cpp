#include "GAS/ActionPracticeAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/ActionPracticeAbilitySystemComponent.h"

UActionPracticeAttributeSet::UActionPracticeAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	InitStaminaRegenRate(10.0f);
	InitDefense(10.0f);
	InitStrength(10.0f);
	InitDexterity(10.0f);
	InitPhysicalAttackPower(50.0f);
	InitMovementSpeed(600.0f);
}

void UActionPracticeAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, StaminaRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, Dexterity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, PhysicalAttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UActionPracticeAttributeSet, MovementSpeed, COND_None, REPNOTIFY_Always);
}

void UActionPracticeAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetStaminaRegenRateAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetDefenseAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetStrengthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); // 최소 1
	}
	else if (Attribute == GetDexterityAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f); // 최소 1
	}
	else if (Attribute == GetMovementSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);

		if (AActor* OwnerActor = GetOwningActor())
		{
			if (ACharacter* Character = Cast<ACharacter>(OwnerActor))
			{
				if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
				{
					MovementComp->MaxWalkSpeed = NewValue;
				}
			}
		}
	}
}

void UActionPracticeAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	 //Target: 현재 AttributeSet이 변경된(이 PostGameplayEffectExecute가 실행된) 목표의 ASC
	 //Source: 적용된 GE spec을 만든 소스의 ASC
	
	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
	const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();

	AActor* TargetActor = nullptr;
	AController* TargetController = nullptr;
	ACharacter* TargetCharacter = nullptr;
	
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		TargetCharacter = Cast<ACharacter>(TargetActor);
	}

	//피격 대미지 처리
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		//소스 액터
		AActor* SourceActor = nullptr;
		AController* SourceController = nullptr;
		if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
		{
			SourceActor = Source->AbilityActorInfo->AvatarActor.Get();
			SourceController = Source->AbilityActorInfo->PlayerController.Get();
		}

		// Store a local copy of the amount of damage done and clear the damage attribute
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0.f);

		if (LocalIncomingDamage > 0)
		{
			// Apply defense calculation (엘든링 스타일 방어력 계산)
			const float DefenseReduction = GetDefense() / (GetDefense() + 100.0f);
			const float FinalDamage = LocalIncomingDamage * (1.0f - DefenseReduction);
			
			// Apply the health change
			const float OldHealth = GetHealth();
			SetHealth(FMath::Clamp(OldHealth - FinalDamage, 0.0f, GetMaxHealth()));

			// Handle death
			if (GetHealth() <= 0.0f)
			{
				// TODO: Implement death logic
			}
		}
	}
	//체력회복 처리
	else if (Data.EvaluatedData.Attribute == GetIncomingHealingAttribute())
	{
		const float LocalIncomingHealing = GetIncomingHealing();
		SetIncomingHealing(0.f);

		if (LocalIncomingHealing > 0)
		{
			const float OldHealth = GetHealth();
			SetHealth(FMath::Clamp(OldHealth + LocalIncomingHealing, 0.0f, GetMaxHealth()));
		}
	}

	//Secondary Attribute를 변경하는 것들 처리
	else if (Data.EvaluatedData.Attribute == GetStrengthAttribute() || 
			 Data.EvaluatedData.Attribute == GetDexterityAttribute())
	{
		CalculateSecondaryAttributes();
	}
	else if (Data.EvaluatedData.Attribute == GetMovementSpeedAttribute())
	{
		if (TargetCharacter && TargetCharacter->GetCharacterMovement())
		{
			TargetCharacter->GetCharacterMovement()->MaxWalkSpeed = GetMovementSpeed();
		}
	}
	
	//기본 Attribute 처리
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		AdjustAttributeForMaxChange(Health, MaxHealth, GetMaxHealth(), GetHealthAttribute());
	}
	else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		AdjustAttributeForMaxChange(Stamina, MaxStamina, GetMaxStamina(), GetStaminaAttribute());
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

float UActionPracticeAttributeSet::GetHealthPercent() const
{
	return GetMaxHealth() > 0.0f ? GetHealth() / GetMaxHealth() : 0.0f;
}

float UActionPracticeAttributeSet::GetStaminaPercent() const
{
	return GetMaxStamina() > 0.0f ? GetStamina() / GetMaxStamina() : 0.0f;
}

float UActionPracticeAttributeSet::CalculateWeaponDamageBonus(float StrengthScaling, float DexterityScaling) const
{
	// 엘든링 스타일 무기 스케일링 계산
	const float StrengthBonus = GetStrength() * StrengthScaling * 0.01f; // 스케일링을 퍼센트로 계산
	const float DexterityBonus = GetDexterity() * DexterityScaling * 0.01f;
	
	return StrengthBonus + DexterityBonus;
}

// Rep Notify Functions
void UActionPracticeAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, Health, OldHealth);
}

void UActionPracticeAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, MaxHealth, OldMaxHealth);
}

void UActionPracticeAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, Stamina, OldStamina);
}

void UActionPracticeAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, MaxStamina, OldMaxStamina);
}

void UActionPracticeAttributeSet::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, StaminaRegenRate, OldStaminaRegenRate);
}

void UActionPracticeAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldDefense)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, Defense, OldDefense);
}

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

void UActionPracticeAttributeSet::OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UActionPracticeAttributeSet, MovementSpeed, OldMovementSpeed);
}

void UActionPracticeAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}