#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

UENUM(BlueprintType)
enum class EWeaponEnums : uint8
{
	None UMETA(DisplayName = "None"), 
	StraightSword UMETA(DisplayName = "StraightSword"), 
	GreatSword UMETA(DisplayName = "GreatSword"),
	Shield UMETA(DisplayName = "Shield"),
};

UENUM(BlueprintType)
enum class EAttackDamageType : uint8
{
	None UMETA(DisplayName = "None"), 
	Slash UMETA(DisplayName = "Slash"), 
	Strike UMETA(DisplayName = "Strike"), 
	Pierce UMETA(DisplayName = "Pierce"), 
};