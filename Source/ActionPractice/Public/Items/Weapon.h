#pragma once

#include "Public/Items/WeaponEnums.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class AActionPracticeCharacter;
class UWeaponDataAsset;
class UStaticMeshComponent;
class UPrimitiveComponent;
class UWeaponCollisionComponent;  
struct FGameplayTag;
struct FBlockActionData;
struct FAttackActionData;

UCLASS()
class AWeapon : public AActor
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"
	
#pragma endregion

#pragma region "Public Functions"
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	
	// ===== Getter =====
	FORCEINLINE FString GetWeaponName() const {return WeaponName;}
	EWeaponEnums GetWeaponType() const;

	FORCEINLINE AActionPracticeCharacter* GetOwnerCharacter() const {return OwnerCharacter;}
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
	const UWeaponDataAsset* GetWeaponData() const { return WeaponData.Get(); }
	
	const FBlockActionData* GetWeaponBlockData() const;
	const FAttackActionData* GetWeaponAttackDataByTag(FGameplayTag AttackTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon")
	UWeaponCollisionComponent* GetCollisionComponent() const { return CollisionComponent; }
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void EquipWeapon();	

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
#pragma endregion

protected:
#pragma region "Protected Variables"
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponCollisionComponent* CollisionComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	FString WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
	TObjectPtr<UWeaponDataAsset> WeaponData;
	
#pragma endregion

#pragma region "Protected Functions"
	
	UFUNCTION()
	void HandleWeaponHit(AActor* HitActor, const FHitResult& HitResult, EAttackDamageType DamageType, float DamageMultiplier);
	
#pragma endregion
	
#pragma endregion

private:
#pragma region "Private Variables"

	UPROPERTY()
	TObjectPtr<AActionPracticeCharacter> OwnerCharacter;
	
#pragma endregion

#pragma region "Private Functions"
	
#pragma endregion
};