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
	UActionPracticeGameplayAbility();

protected:
	// 기본 클래스의 태그들을 사용하므로 여기서는 제거

	// 스태미나 비용
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Costs")
	float StaminaCost = 0.0f;

	// 쿨다운 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cooldown")
	float CooldownDuration = 0.0f;

	// 어빌리티 레벨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	int32 AbilityLevel = 1;

public:
	// 어빌리티 초기화
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	// 어빌리티 활성화 가능 여부 확인
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// 스태미나 체크
	UFUNCTION(BlueprintPure, Category = "Ability")
	virtual bool CheckStaminaCost() const;

	// 스태미나 소모
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool ConsumeStamina();

	// 어빌리티 활성화
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 어빌리티 종료
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 캐릭터 레퍼런스 가져오기
	UFUNCTION(BlueprintPure, Category = "Ability")
	class AActionPracticeCharacter* GetActionPracticeCharacterFromActorInfo() const;

	// AttributeSet 레퍼런스 가져오기
	UFUNCTION(BlueprintPure, Category = "Ability")
	class UActionPracticeAttributeSet* GetActionPracticeAttributeSetFromActorInfo() const;
};