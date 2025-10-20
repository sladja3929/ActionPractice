#pragma once

struct FBlockActionData;
struct FAttackActionData;
class UGameplayAbility;
class AWeapon;

class FWeaponAbilityStatics
{
public:
	static AWeapon* GetWeaponFromAbility(const UGameplayAbility* Ability, bool bIsLeft);
	static const FAttackActionData* GetAttackDataFromAbility(const UGameplayAbility* Ability);
	static const FBlockActionData* GetBlockDataFromAbility(const UGameplayAbility* Ability);
};
