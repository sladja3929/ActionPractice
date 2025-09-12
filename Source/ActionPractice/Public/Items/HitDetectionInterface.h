#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "HitDetectionInterface.generated.h"

struct FGameplayEventData;

UINTERFACE(Blueprintable)
class ACTIONPRACTICE_API UHitDetectionInterface : public UInterface
{
	GENERATED_BODY()
};

class ACTIONPRACTICE_API IHitDetectionInterface
{
	GENERATED_BODY()
	
public:
	//virtual ~IHitDetectionInterface() = default;

	virtual void PrepareHitDetection(const FGameplayTagContainer& AttackTags, const int32 ComboIndex) = 0;
	virtual void HandleHitDetectionStart(const FGameplayEventData& Payload) = 0;
	virtual void HandleHitDetectionEnd(const FGameplayEventData& Payload) = 0;
};