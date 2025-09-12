#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "InputActionDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FInputActionAbilityRule
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTagContainer AbilityAssetTags;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Buffer")
    bool bCanBuffered = false;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Buffer")
    int BufferPriority = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Buffer")
    bool bIsHoldAction = false;
};

UCLASS(BlueprintType)
class UInputActionDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TMap<TObjectPtr<UInputAction>, FInputActionAbilityRule> Rules;
    
    const FInputActionAbilityRule* FindRuleByAction(const UInputAction* InputAction) const
    {
        if (!InputAction)
        {
            UE_LOG(LogTemp, Warning, TEXT("FindRuleByAction: InAction is null"));
            return nullptr;
        }

        if (const FInputActionAbilityRule* Found = Rules.Find(InputAction))
        {
            return Found;
        }

        UE_LOG(LogTemp, Warning, TEXT("FindRuleByAction: Rule not found for Action=%s"), *GetNameSafe(InputAction));
        return nullptr;
    }
};