#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsSubsystem.generated.h"

class UGameplayTagsDataAsset;

UCLASS()
class ACTIONPRACTICE_API UGameplayTagsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Static 게임플레이 태그 접근 함수들
	
	// Ability Tags
	static const FGameplayTag& GetAbilityAttackTag();
	static const FGameplayTag& GetAbilityRollTag();
	static const FGameplayTag& GetAbilitySprintTag();
	static const FGameplayTag& GetAbilityJumpTag();
	static const FGameplayTag& GetAbilityBlockTag();
	
	// State Tags
	static const FGameplayTag& GetStateAttackingTag();
	static const FGameplayTag& GetStateBlockingTag();
	static const FGameplayTag& GetStateRecoveringTag();
	static const FGameplayTag& GetStateStunnedTag();
	static const FGameplayTag& GetStateInvincibleTag();
	
	// Event Tags
	static const FGameplayTag& GetEventNotifyEnableComboInputTag();
	static const FGameplayTag& GetEventNotifyActionRecoveryEndTag();
	static const FGameplayTag& GetEventNotifyResetComboTag();

private:
	// Internal helper function
	static UGameplayTagsSubsystem* Get();

	// 게임플레이 태그 접근 함수들 (인스턴스 버전)
	
	// Ability Tags
	const FGameplayTag& GetAbilityAttackTagInternal() const;
	const FGameplayTag& GetAbilityRollTagInternal() const;
	const FGameplayTag& GetAbilitySprintTagInternal() const;
	const FGameplayTag& GetAbilityJumpTagInternal() const;
	const FGameplayTag& GetAbilityBlockTagInternal() const;
	
	// State Tags
	const FGameplayTag& GetStateAttackingTagInternal() const;
	const FGameplayTag& GetStateBlockingTagInternal() const;
	const FGameplayTag& GetStateRecoveringTagInternal() const;
	const FGameplayTag& GetStateStunnedTagInternal() const;
	const FGameplayTag& GetStateInvincibleTagInternal() const;
	
	// Event Tags
	const FGameplayTag& GetEventNotifyEnableComboInputTagInternal() const;
	const FGameplayTag& GetEventNotifyActionRecoveryEndTagInternal() const;
	const FGameplayTag& GetEventNotifyResetComboTagInternal() const;

protected:
	// 태그 데이터 에셋
	UPROPERTY()
	TObjectPtr<UGameplayTagsDataAsset> GameplayTagsDataAsset;
};