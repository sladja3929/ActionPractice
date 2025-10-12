#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "AttackData.generated.h"

UENUM(BlueprintType)
enum class EAttackDamageType : uint8
{
	None UMETA(DisplayName = "None"),
	Slash UMETA(DisplayName = "Slash"),
	Strike UMETA(DisplayName = "Strike"),
	Pierce UMETA(DisplayName = "Pierce")
};

USTRUCT(BlueprintType)
struct FFinalAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	EAttackDamageType DamageType = EAttackDamageType::None;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float FinalDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float StaminaDamage = 10.0f;
};