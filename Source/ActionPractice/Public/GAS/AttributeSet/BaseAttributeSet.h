#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BaseAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class ACTIONPRACTICE_API UBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"
	
	// ===== Base Attributes =====

	//HP
	UPROPERTY(BlueprintReadOnly, Category = "Vital", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Vital", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MaxHealth)

	//Stamina
	UPROPERTY(BlueprintReadOnly, Category = "Vital", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, Category = "Vital", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, Category = "Vital", ReplicatedUsing = OnRep_StaminaRegenRate)
	FGameplayAttributeData StaminaRegenRate;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, StaminaRegenRate)

	//Defense
	UPROPERTY(BlueprintReadOnly, Category = "Combat", ReplicatedUsing = OnRep_Defense)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, Defense)
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing = OnRep_MovementSpeed)
	FGameplayAttributeData MovementSpeed;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MovementSpeed)

	// ===== Meta Attributes =====
	
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, IncomingDamage)
	
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData IncomingHealing;
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, IncomingHealing)
	
#pragma endregion

#pragma region "Public Functions"
	UBaseAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//Duration, Infinite에서 수행
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	//Instant, Periodic에서 수행
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	//GE 직후 Instant, Periodic에서 수행
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	//Helper functions for calculations
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetStaminaPercent() const;
	
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
	virtual void OnRep_MovementSpeed(const FGameplayAttributeData& OldMovementSpeed);

	//Helper function to adjust attributes when max value changes
	void AdjustAttributeForMaxChange(const FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const;
	
#pragma endregion
};