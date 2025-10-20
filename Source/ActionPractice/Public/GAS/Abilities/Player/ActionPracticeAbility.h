#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BaseAbility.h"
#include "GameplayTagContainer.h"
#include "ActionPracticeAbility.generated.h"

class UActionPracticeAbilitySystemComponent;
class UInputBufferComponent;
class UActionPracticeAttributeSet;
class AActionPracticeCharacter;

UCLASS()
class ACTIONPRACTICE_API UActionPracticeAbility : public UBaseAbility
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"

#pragma endregion

#pragma region "Public Functions"

	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void ActivateInitSettings() override;

#pragma endregion

protected:
#pragma region "Protected Variables"

#pragma endregion

#pragma region "Protected Functions"

	UFUNCTION(BlueprintPure, Category = "Ability")
	UActionPracticeAbilitySystemComponent* GetActionPracticeAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	AActionPracticeCharacter* GetActionPracticeCharacterFromActorInfo() const;

	//멤버변수 Actor Info 활성화 전 사용 (CanActivateAbility 등)
	AActionPracticeCharacter* GetActionPracticeCharacterFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	UActionPracticeAttributeSet* GetActionPracticeAttributeSetFromActorInfo() const;

	//멤버변수 Actor Info 활성화 전 사용 (CanActivateAbility 등)
	UActionPracticeAttributeSet* GetActionPracticeAttributeSetFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	UInputBufferComponent* GetInputBufferComponentFromActorInfo() const;

#pragma endregion
};