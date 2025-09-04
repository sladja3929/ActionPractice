#include "GAS/Abilities/WeaponAbilityStatics.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "Items/Weapon.h"
#include "Items/WeaponData.h"


AWeapon* FWeaponAbilityStatics::GetWeaponFromAbility(const UGameplayAbility* Ability, bool bIsLeft)
{
	AActionPracticeCharacter* Character = Cast<AActionPracticeCharacter>(Ability->GetActorInfo().AvatarActor.Get());
	return bIsLeft ? Character->GetLeftWeapon() : Character->GetRightWeapon();
}

const FAttackActionData* FWeaponAbilityStatics::GetAttackDataFromAbility(const UGameplayAbility* Ability)
{
	AWeapon* Weapon = GetWeaponFromAbility(Ability, false);
	if (!Weapon) return nullptr;

	FGameplayTag MainTag = Ability->AbilityTags.First();
	if (!MainTag.IsValid()) return nullptr;

	// 몽타주 검증, 데이터 배열 크기 일치 검증
	const FAttackActionData* WeaponAttackData = Weapon->GetWeaponAttackDataByTag(MainTag);
	if (!WeaponAttackData) return nullptr;
	if (WeaponAttackData->AttackMontages.IsEmpty() || !WeaponAttackData->AttackMontages[0]) return nullptr;
	if (WeaponAttackData->AttackMontages.Num() != WeaponAttackData->ComboAttackData.Num()) return nullptr;

	return WeaponAttackData;
}

const FBlockActionData* FWeaponAbilityStatics::GetBlockDataFromAbility(const UGameplayAbility* Ability)
{
	AWeapon* Weapon = GetWeaponFromAbility(Ability, true);
	if (!Weapon) return nullptr;
	
	//몽타주 검증
	const FBlockActionData* WeaponBlockData = Weapon->GetWeaponBlockData();
	if (!WeaponBlockData) return nullptr;
	if (!WeaponBlockData->BlockIdleMontage || !WeaponBlockData->BlockReactionMontage) return nullptr;
	
	return WeaponBlockData;
}
