#pragma once

#include "Public/Items/WeaponEnums.h"
#include "Public/Items/WeaponData.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Weapon.generated.h"

UCLASS()
class AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	
	// ===== Info Getter =====
	FORCEINLINE FString GetWeaponName() const {return WeaponName;}
	FORCEINLINE WeaponEnums GetWeaponType() const {return WeaponType;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
	const UWeaponDataAsset* GetWeaponData() const { return WeaponData.Get(); }
	const FBlockActionData* GetWeaponBlockData() const;
	const FAttackActionData* GetWeaponAttackDataByTag(FGameplayTag AttackTag) const;

	
	// 무기 사용 함수
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void EquipWeapon();

	// ===== Combat Calculation Functions =====
	/*UFUNCTION(BlueprintPure, Category = "Combat")
	float CalculateTotalAttackPower(float PlayerStrength, float PlayerDexterity) const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CheckStatRequirements(float PlayerStrength, float PlayerDexterity) const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetAttackPowerPenalty(float PlayerStrength, float PlayerDexterity) const;*/

	// 콜리전 이벤트 함수들
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WeaponMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Info")
	FString WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Info")
	WeaponEnums WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	TObjectPtr<UWeaponDataAsset> WeaponData;
};