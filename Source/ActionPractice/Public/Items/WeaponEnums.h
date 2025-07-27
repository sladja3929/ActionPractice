#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

UENUM(BlueprintType)
enum class WeaponEnums : uint8
{
	StraightSword UMETA(DisplayName = "StraightSword"), 
	GreatSword UMETA(DisplayName = "GreatSword"),
	Shield UMETA(DisplayName = "Shield"),
};
