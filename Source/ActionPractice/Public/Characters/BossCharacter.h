#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "BossCharacter.generated.h"

class UBossAttributeSet;

UCLASS()
class ACTIONPRACTICE_API ABossCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"


#pragma endregion

#pragma region "Public Functions"

	ABossCharacter();

	//===== Hit Detection Interface =====
	virtual TScriptInterface<IHitDetectionInterface> GetHitDetectionInterface() const override;

#pragma endregion

protected:
#pragma region "Protected Variables"


#pragma endregion

#pragma region "Protected Functions"

	//===== GAS =====
	virtual void CreateAbilitySystemComponent() override;
	virtual void CreateAttributeSet() override;

#pragma endregion

private:
#pragma region "Private Variables"


#pragma endregion

#pragma region "Private Functions"


#pragma endregion
};