#pragma once

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

protected:
	virtual void BeginPlay() override;

	// 무기 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* WeaponMesh;

	// 무기 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	float Damage;

	// 무기 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Stats")
	FString WeaponName;

public:
	virtual void Tick(float DeltaTime) override;

	// 무기 사용 함수
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void UseWeapon();

	// 콜리전 이벤트 함수들
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Getter 함수들
	FORCEINLINE float GetDamage() const 
	{ 
		return Damage; 
	}
    
	FORCEINLINE FString GetWeaponName() const 
	{ 
		return WeaponName; 
	}
};