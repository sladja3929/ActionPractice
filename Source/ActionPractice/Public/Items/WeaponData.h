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
	
	// 콤보별 몽타주 배열
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TArray<TSoftObjectPtr<UAnimMontage>> AttackMontages;

	// 콤보별 공격 데이터 (AttackMontages와 배열 크기가 같아야 함)
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
	
	// WeaponData가 로드될 때 자동으로 몽타주들을 프리로딩
	virtual void PostLoad() override
	{
		Super::PostLoad();
		
		// 에디터에서는 프리뷰 월드 체크로 애니메이션 에디터 제외
		UWorld* World = GetWorld();
		if (!World || World->IsPreviewWorld())
		{
			return;
		}
		
		// 모든 공격 몽타주 프리로딩
		for (auto& AttackDataPair : AttackDataMap)
		{
			FAttackActionData& AttackData = AttackDataPair.Value;
			for (TSoftObjectPtr<UAnimMontage>& SoftMontage : AttackData.AttackMontages)
			{
				if (!SoftMontage.IsNull() && !SoftMontage.IsValid())
				{
					(void)SoftMontage.LoadSynchronous();
				}
			}
		}
		
		// 블록 몽타주 프리로딩
		if (!BlockData.BlockMontage.IsNull() && !BlockData.BlockMontage.IsValid())
		{
			(void)BlockData.BlockMontage.LoadSynchronous();
		}
	}

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