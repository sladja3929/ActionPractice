#pragma once

#include "UObject/Interface.h"
#include "MontageAbilityInterface.generated.h"

UINTERFACE()
class UMontageAbilityInterface : public UInterface
{
	GENERATED_BODY()
};

class ACTIONPRACTICE_API IMontageAbilityInterface
{
	GENERATED_BODY()
	
public:

	virtual void PlayAction() = 0;

	virtual UAnimMontage* SetMontageToPlayTask() = 0;

	virtual void BindEventsAndReadyMontageTask() = 0;
	
	virtual void ExecuteMontageTask() = 0;

	virtual void OnTaskMontageCompleted() = 0;

	virtual void OnTaskMontageInterrupted() = 0;
};