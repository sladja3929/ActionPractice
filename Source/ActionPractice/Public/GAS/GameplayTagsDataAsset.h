#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsDataAsset.generated.h"

UCLASS(BlueprintType)
class ACTIONPRACTICE_API UGameplayTagsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#pragma region "Ability Tags"
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Tags")
	FGameplayTag Ability_Attack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Tags")
	FGameplayTag Ability_Roll;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Tags")
	FGameplayTag Ability_Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Tags")
	FGameplayTag Ability_Jump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Tags")
	FGameplayTag Ability_Block
	;
#pragma endregion

#pragma region "State Tags"

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State Tags")
	FGameplayTag State_Attacking;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State Tags")
	FGameplayTag State_Blocking;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State Tags")
	FGameplayTag State_Recovering;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State Tags")
	FGameplayTag State_Stunned;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State Tags")
	FGameplayTag State_Invincible;
	
#pragma endregion

#pragma region "Event Tags"

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event Tags")
	FGameplayTag Event_Notify_EnableComboInput;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event Tags")
	FGameplayTag Event_Notify_ActionRecoveryEnd;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event Tags")
	FGameplayTag Event_Notify_ResetCombo;
	
#pragma endregion
};