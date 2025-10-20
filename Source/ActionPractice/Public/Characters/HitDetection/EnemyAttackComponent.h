// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/HitDetection/AttackTraceComponent.h"
#include "EnemyAttackComponent.generated.h"

class ABossCharacter;
class USkeletalMeshComponent;
class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ACTIONPRACTICE_API UEnemyAttackComponent : public UAttackTraceComponent
{
	GENERATED_BODY()

public:
#pragma region "Public Functions"

	UEnemyAttackComponent();

	virtual void BeginPlay() override;

	virtual AActor* GetOwnerActor() const override;
	virtual UAbilitySystemComponent* GetOwnerASC() const override;

#pragma endregion

protected:
#pragma region "Protected Variables"

	UPROPERTY()
	TObjectPtr<ABossCharacter> OwnerEnemy = nullptr;

#pragma endregion

#pragma region "Protected Functions"

	// EnemyData에서 트레이스 설정 로드 (TODO: 나중에 EnemyDataAsset 생성 후 구현)
	virtual bool LoadTraceConfig(const FGameplayTagContainer& AttackTags, int32 ComboIndex) override;

	virtual void SetOwnerMesh() override;

	virtual void AddIgnoredActors(FCollisionQueryParams& Params) const override;

#pragma endregion
};