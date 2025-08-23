#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsDataAsset.generated.h"

// 태그를 추가하면 에디터의 데이터 에셋에서 꼭 실제 태그를 바인딩할 것
UCLASS(BlueprintType)
class ACTIONPRACTICE_API UGameplayTagsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
#pragma region "Ability Tags"
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Tags")
	FGameplayTag Ability_Attack_Normal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Tags")
	FGameplayTag Ability_Attack_Charge;
	
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
	FGameplayTag State_Sprinting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "State Tags")
	FGameplayTag State_Jumping;
	
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