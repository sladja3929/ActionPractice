#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "ActionPracticeGameplayAbility.generated.h"

UCLASS()
class ACTIONPRACTICE_API UActionPracticeGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"
	
#pragma endregion

#pragma region "Public Functions"
	
	UActionPracticeGameplayAbility();
	
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	// 어빌리티 활성화 가능 여부 확인 (이 함수에서 호출하는 함수는 무조건 파라미터 ActorInfo를 넘겨받아 사용해야 함, Instance Policing에 따라 에러날 수 있음)
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// 스태미나 체크 (UFUNCTION은 원시 포인터 *를 인자로 못받기 때문에 참조 사용)
	UFUNCTION(BlueprintPure, Category = "Ability")
	virtual bool CheckStaminaCost(const FGameplayAbilityActorInfo& ActorInfo) const;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool ApplyStaminaCost();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

#pragma endregion
	
protected:
#pragma region "Protected Variables"
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Stats")
	float StaminaCost = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Stats")
	float CooldownDuration = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Stats")
	int32 AbilityLevel = 1;

	//스테미나 사용 관련
	UPROPERTY(EditDefaultsOnly, Category="Cost")
	TSubclassOf<class UGameplayEffect> StaminaCostEffect;
	
	FGameplayTag EffectStaminaCostTag;
	
#pragma endregion
	
#pragma region "Protected Functions"
	
	UFUNCTION(BlueprintPure, Category = "Ability")
	class AActionPracticeCharacter* GetActionPracticeCharacterFromActorInfo() const;

	//멤버변수 Actor Info 활성화 전 사용 (CanActivateAbility 등)
	class AActionPracticeCharacter* GetActionPracticeCharacterFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const;
	
	UFUNCTION(BlueprintPure, Category = "Ability")
	class UActionPracticeAttributeSet* GetActionPracticeAttributeSetFromActorInfo() const;

	//멤버변수 Actor Info 활성화 전 사용 (CanActivateAbility 등)
	class UActionPracticeAttributeSet* GetActionPracticeAttributeSetFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	class UInputBufferComponent* GetInputBufferComponentFromActorInfo() const;
	
#pragma endregion
};