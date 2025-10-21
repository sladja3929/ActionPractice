// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/HitDetection/AttackTraceComponent.h"
#include "WeaponAttackComponent.generated.h"

class AWeapon;
class UStaticMeshComponent;
class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ACTIONPRACTICE_API UWeaponAttackComponent : public UAttackTraceComponent
{
	GENERATED_BODY()

public:
#pragma region "Public Functions"

	UWeaponAttackComponent();

	virtual void BeginPlay() override;

	virtual AActor* GetOwnerActor() const override;
	virtual UAbilitySystemComponent* GetOwnerASC() const override;
	
#pragma endregion

protected:
#pragma region "Protected Variables"

	UPROPERTY()
	TObjectPtr<AWeapon> OwnerWeapon = nullptr;

#pragma endregion

#pragma region "Protected Functions"

	// WeaponData에서 트레이스 설정 로드
	virtual bool LoadTraceConfig(const FGameplayTagContainer& AttackTags, int32 ComboIndex) override;

	virtual void SetOwnerMesh() override;

	virtual void AddIgnoredActors(FCollisionQueryParams& Params) const override;

#pragma endregion
};
