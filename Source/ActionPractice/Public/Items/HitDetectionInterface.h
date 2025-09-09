#pragma once

#include "HitDetectionInterface.generated.h"

UINTERFACE()
class UHitDetectionInterface : public UInterface
{
	GENERATED_BODY()
};

class ACTIONPRACTICE_API IHitDetectionInterface
{
	GENERATED_BODY()
	
public:

	virtual void PrepareHitDetection(const FGameplayTag& AttackTag, const int32 ComboIndex) = 0;
	virtual void HandleHitDetectionStart(const FGameplayEventData& Payload) = 0;
	virtual void HandleHitDetectionEnd(const FGameplayEventData& Payload) = 0;
};