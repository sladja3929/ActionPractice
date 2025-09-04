#pragma once

#include "MontageAbility.generated.h"

UINTERFACE()
class UMontageAbility : public UInterface
{
	GENERATED_BODY()
};

class ACTIONPRACTICE_API IMontageAbility
{
	GENERATED_BODY()
	
public:

	virtual void PlayAction() = 0;

	virtual void ExecuteMontageTask() = 0;

	virtual void OnTaskMontageCompleted() = 0;

	virtual void OnTaskMontageInterrupted() = 0;
};