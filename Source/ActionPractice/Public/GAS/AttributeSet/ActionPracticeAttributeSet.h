#pragma once

#include "CoreMinimal.h"
#include "GAS/AttributeSet/BaseAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ActionPracticeAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class ACTIONPRACTICE_API UActionPracticeAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vital")
	float StaminaDepletionDuration = 0.5f;

	// ===== Primary Attributes =====

	//Strength 근력
	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, Strength)

	//Dexterity 기력
	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing = OnRep_Dexterity)
	FGameplayAttributeData Dexterity;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, Dexterity)

	// ===== Secondary Attributes (Primary 기반 계산된 능력치) =====

	// Physical Attack Power (물리 공격력)
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_PhysicalAttackPower)
	FGameplayAttributeData PhysicalAttackPower;
	ATTRIBUTE_ACCESSORS(UActionPracticeAttributeSet, PhysicalAttackPower)
#pragma endregion

#pragma region "Public Functions"
	UActionPracticeAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//Duration, Infinite에서 수행
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//Instant, Periodic에서 수행
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	//GE 직후 Instant, Periodic에서 수행
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	//Helper functions for calculations
	UFUNCTION(BlueprintPure, Category = "Combat")
	float CalculateWeaponDamageBonus(float StrengthScaling, float DexterityScaling) const;
#pragma endregion

protected:
#pragma region "Protected Functions"
	UFUNCTION()
	virtual void OnRep_Strength(const FGameplayAttributeData& OldStrength);

	UFUNCTION()
	virtual void OnRep_Dexterity(const FGameplayAttributeData& OldDexterity);

	UFUNCTION()
	virtual void OnRep_PhysicalAttackPower(const FGameplayAttributeData& OldPhysicalAttackPower);

	//Calculate secondary attributes from primary stats
	void CalculateSecondaryAttributes();
#pragma endregion
};