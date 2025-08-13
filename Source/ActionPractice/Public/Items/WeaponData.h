#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponData.generated.h"

// UAnimMontage 클래스를 직접 포함하는 대신 전방 선언하여 컴파일 의존성을 낮춥니다.
class UAnimMontage;


//개별 공격 데이터
USTRUCT(BlueprintType)
struct FIndividualAttackData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float StaminaDamage = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float StaminaCost = 10.0f;
	// 필요에 따라 경직도, 사운드, 파티클 이펙트 등의 데이터를 여기에 추가할 수 있습니다.
};

//공격 유형 하나에 대한 정보
USTRUCT(BlueprintType)
struct FAttackActionData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> AttackMontage;

	//MaxComboCount는 배열 크기로
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TArray<FIndividualAttackData> ComboAttackData;
};

//방어 정보
USTRUCT(BlueprintType)
struct FBlockActionData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TSoftObjectPtr<UAnimMontage> BlockMontage;

	//방어 데미지 감소량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float DamageReduction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
	float StaminaCost = 10.0f;
};

UCLASS(BlueprintType)
class UWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Stats")
	float BaseDamage = 100.0f;

	// 근력 보정 (A, B, C, D, E 등급을 숫자로 표현: 80=A, 60=B, 40=C, 20=D, 0=E)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float StrengthScaling;
	
	// 기량 보정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Base Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float DexterityScaling;
	
	//GameplayTag를 Key로 사용하여 각 공격 타입에 맞는 데이터를 쉽게 찾을 수 있습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Definitions")
	TMap<FGameplayTag, FAttackActionData> AttackDataMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Definitions")
	FBlockActionData BlockData;
};