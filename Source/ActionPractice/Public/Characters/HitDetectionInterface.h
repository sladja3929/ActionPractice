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

	virtual void PrepareHitDetection(const FGameplayTag& AttackTag) = 0;
};