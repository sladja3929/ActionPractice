#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Items/WeaponEnums.h"
#include "ActionPracticeGameplayEffectContext.generated.h"

USTRUCT()
struct ACTIONPRACTICE_API FActionPracticeGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FActionPracticeGameplayEffectContext(): Super(), AttackDamageType(EAttackDamageType::None) {}
	virtual ~FActionPracticeGameplayEffectContext()	{}

	//오버라이드 필수 Functions
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
	virtual FGameplayEffectContext* Duplicate() const override;
	virtual UScriptStruct* GetScriptStruct() const override;

	//Getter
	EAttackDamageType GetAttackDamageType() const {	return AttackDamageType; }

	//Setter
	void SetAttackDamageType(EAttackDamageType InAttackDamageType) { AttackDamageType = InAttackDamageType;	}

	//다운캐스팅 헬퍼 함수
	static FActionPracticeGameplayEffectContext* GetActionPracticeEffectContext(FGameplayEffectContextHandle& Handle);
	static const FActionPracticeGameplayEffectContext* GetActionPracticeEffectContext(const FGameplayEffectContextHandle& Handle);

protected:
	
	UPROPERTY()
	EAttackDamageType AttackDamageType;

};

template<>
struct TStructOpsTypeTraits<FActionPracticeGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FActionPracticeGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};