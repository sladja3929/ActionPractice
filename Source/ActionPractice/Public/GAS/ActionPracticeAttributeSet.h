#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ActionPracticeAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class ACTIONPRACTICE_API UActionPracticeAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"
	// ===== Standard Attributes =====

	//HP
	UPROPERTY(BlueprintReadOnly, Category = "Vital", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Vital", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, MaxHealth)

	//Stamina
	UPROPERTY(BlueprintReadOnly, Category = "Vital", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Vital", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, Category = "Vital", ReplicatedUsing = OnRep_StaminaRegenRate)
	FGameplayAttributeData StaminaRegenRate;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, StaminaRegenRate)

	//Defense
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_Defense)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, Defense)

	//Strength 근력
	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, Strength)

	//Dexterity 기력
	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Dexterity)
	FGameplayAttributeData Dexterity;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, Dexterity)

	// ===== Secondary Attributes (계산된 능력치) =====

	// Physical Attack Power (물리 공격력)
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_PhysicalAttackPower)
	FGameplayAttributeData PhysicalAttackPower;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, PhysicalAttackPower)

	// Movement Speed (이동 속도)
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_MovementSpeed)
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, MovementSpeed)

	// ===== Meta Attributes (계산용, 복제되지 않음) =====

	//Incoming Damage
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, IncomingDamage)

	//Incoming Healing
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData IncomingHealing;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, IncomingHealing)
#pragma endregion

#pragma region "Public Functions"
	UActionPracticeAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// Helper functions for calculations
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetStaminaPercent() const;

	// Calculate weapon damage bonus based on stats
	UFUNCTION(BlueprintPure, Category = "Combat")
	float CalculateWeaponDamageBonus(float StrengthScaling, float DexterityScaling) const;
#pragma endregion

protected:
#pragma region "Protected Functions"
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

	UFUNCTION()
	virtual void OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate);

	UFUNCTION()
	virtual void OnRep_Defense(const FGameplayAttributeData& OldDefense);

	UFUNCTION()
	virtual void OnRep_Strength(const FGameplayAttributeData& OldStrength);

	UFUNCTION()
	virtual void OnRep_Dexterity(const FGameplayAttributeData& OldDexterity);

	UFUNCTION()
	virtual void OnRep_PhysicalAttackPower(const FGameplayAttributeData& OldPhysicalAttackPower);

	UFUNCTION()
	virtual void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);

	// Helper function to adjust attributes when max value changes
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

	// Calculate secondary attributes from primary stats
	void CalculateSecondaryAttributes();
#pragma endregion
};