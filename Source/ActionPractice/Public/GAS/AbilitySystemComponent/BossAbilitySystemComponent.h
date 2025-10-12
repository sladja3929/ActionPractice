#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "BossAbilitySystemComponent.generated.h"

class ABossCharacter;
class UBossAttributeSet;

UCLASS()
class ACTIONPRACTICE_API UBossAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"


#pragma endregion

#pragma region "Public Functions"

	UBossAbilitySystemComponent();

	// ASC가 Owner/Avatar 정보를 수집하는 지점: 캐시/델리게이트 바인딩은 여기에서
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	UFUNCTION(BlueprintPure, Category="Attributes")
	const UBossAttributeSet* GetBossAttributeSet() const;

	// 초기화 완료 델리게이트(필요 시 외부에서 바인딩)
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCInitialized, UBossAbilitySystemComponent*);
	FOnASCInitialized OnASCInitialized;

#pragma endregion

protected:
#pragma region "Protected Variables"

	TWeakObjectPtr<ABossCharacter> CachedCharacter;

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