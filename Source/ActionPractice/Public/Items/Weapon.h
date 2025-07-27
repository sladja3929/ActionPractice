#pragma once

#include "Public/Items/WeaponEnums.h"
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

	// ===== Info Getter =====
	FORCEINLINE FString GetWeaponName() const {return WeaponName;}
	FORCEINLINE WeaponEnums GetWeaponType() const {return WeaponType;}
	FORCEINLINE float GetDamage() const {return Damage;}
	FORCEINLINE float GetDamageReduction() const {return DamageReduction;}
	FORCEINLINE float GetShockAbsorption() const {return ShockAbsorption;}
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WeaponMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Info")
	FString WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Base Info")
	WeaponEnums WeaponType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Info")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Info")
	float DamageReduction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Info")
	float ShockAbsorption;


public:
	virtual void Tick(float DeltaTime) override;

	// 무기 사용 함수
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void UseWeapon();

	// 콜리전 이벤트 함수들
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};