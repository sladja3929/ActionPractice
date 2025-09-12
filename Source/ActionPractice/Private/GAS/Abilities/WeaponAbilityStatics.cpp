#include "GAS/Abilities/WeaponAbilityStatics.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "Items/Weapon.h"
#include "Items/WeaponDataAsset.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
	#define DEBUG_LOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)
#else
	#define DEBUG_LOG(Format, ...)
#endif

AWeapon* FWeaponAbilityStatics::GetWeaponFromAbility(const UGameplayAbility* Ability, bool bIsLeft)
{
	AActionPracticeCharacter* Character = Cast<AActionPracticeCharacter>(Ability->GetActorInfo().AvatarActor.Get());
	return bIsLeft ? Character->GetLeftWeapon() : Character->GetRightWeapon();
}

const FAttackActionData* FWeaponAbilityStatics::GetAttackDataFromAbility(const UGameplayAbility* Ability)
{
	AWeapon* Weapon = GetWeaponFromAbility(Ability, false);
	if (!Weapon)
	{
		DEBUG_LOG(TEXT("WeaponAbilityStatics: No Weapon"))
		return nullptr;
	}

	FGameplayTagContainer AssetTag = Ability->GetAssetTags();
	if (AssetTag.IsEmpty())
	{
		DEBUG_LOG(TEXT("WeaponAbilityStatics: No AssetTags"))
		return nullptr;
	}

	// 몽타주 검증, 데이터 배열 크기 일치 검증
	const FAttackActionData* WeaponAttackData = Weapon->GetWeaponAttackDataByTag(AssetTag);
	if (!WeaponAttackData)
	{
		DEBUG_LOG(TEXT("WeaponAbilityStatics: No AttackData"))
		return nullptr;
	}
	
	if (WeaponAttackData->AttackMontages.IsEmpty() || !WeaponAttackData->AttackMontages[0])
	{
		DEBUG_LOG(TEXT("WeaponAbilityStatics: No Attack Montage"))
		return nullptr;
	}
	
	if (WeaponAttackData->AttackMontages.Num() != WeaponAttackData->ComboAttackData.Num())
	{
		DEBUG_LOG(TEXT("WeaponAbilityStatics: Not Same Attack List Size"))
		return nullptr;
	}

	return WeaponAttackData;
}

const FBlockActionData* FWeaponAbilityStatics::GetBlockDataFromAbility(const UGameplayAbility* Ability)
{
	AWeapon* Weapon = GetWeaponFromAbility(Ability, true);
	if (!Weapon)
	{
		DEBUG_LOG(TEXT("WeaponAbilityStatics: No Weapon"))
		return nullptr;
	}
	
	//몽타주 검증
	const FBlockActionData* WeaponBlockData = Weapon->GetWeaponBlockData();
	if (!WeaponBlockData)
	{
		DEBUG_LOG(TEXT("WeaponAbilityStatics: No BlockData"))
		return nullptr;
	}
	
	if (!WeaponBlockData->BlockIdleMontage || !WeaponBlockData->BlockReactionMontage)
	{
		DEBUG_LOG(TEXT("WeaponAbilityStatics: No Block Montages"))
		return nullptr;
	}
	
	return WeaponBlockData;
}
