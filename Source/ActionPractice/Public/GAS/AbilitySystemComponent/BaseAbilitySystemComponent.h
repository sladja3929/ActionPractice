#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "BaseAbilitySystemComponent.generated.h"

class ABaseCharacter;

/**
 * Base AbilitySystemComponent
 * ActionPracticeAbilitySystemComponent와 BossAbilitySystemComponent의 공통 기능
 */
UCLASS()
class ACTIONPRACTICE_API UBaseAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
#pragma region "Public Functions"

	UBaseAbilitySystemComponent();

	//ASC가 Owner/Avatar 정보를 수집하는 지점
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	//초기화 완료 델리게이트(필요 시 외부에서 바인딩)
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCInitialized, UBaseAbilitySystemComponent*);
	FOnASCInitialized OnASCInitialized;

#pragma endregion

protected:
#pragma region "Protected Variables"

	TWeakObjectPtr<ABaseCharacter> CachedCharacter;

#pragma endregion

#pragma region "Protected Functions"

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma endregion

private:
#pragma region "Private Variables"

#pragma endregion

#pragma region "Private Functions"

#pragma endregion
};