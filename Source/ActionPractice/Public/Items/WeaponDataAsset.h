#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponEnums.h"
#include "AttackData.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "WeaponDataAsset.generated.h"

class UAnimMontage;

//개별 공격 데이터
USTRUCT(BlueprintType)
struct FIndividualAttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    EAttackDamageType DamageType = EAttackDamageType::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    float PoiseDamage = 10.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    float StaminaCost = 10.0f;
    //필요에 따라 경직도, 사운드, 파티클 이펙트 등의 데이터를 여기에 추가
};

//공격 유형 하나에 대한 정보
USTRUCT(BlueprintType)
struct FAttackActionData
{
    GENERATED_BODY()
    
    //콤보별 몽타주 배열
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TArray<TSoftObjectPtr<UAnimMontage>> AttackMontages;

    //보조 몽타주 배열 - 추가 액션이 필요할 때만 사용 (차지 액션 등)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TArray<TSoftObjectPtr<UAnimMontage>> SubAttackMontages;
    
    //콤보별 공격 데이터 (AttackMontages와 배열 크기가 같아야 함)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
    TArray<FIndividualAttackData> ComboAttackData;
};

//TMap 대신 사용할 구조체
USTRUCT(BlueprintType)
struct FTaggedAttackData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tags")
    FGameplayTagContainer AttackTags;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    FAttackActionData AttackData;
};

//방어 정보
USTRUCT(BlueprintType)
struct FBlockActionData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> BlockIdleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TSoftObjectPtr<UAnimMontage> BlockReactionMontage;

    //방어 데미지 감소량
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float DamageReduction = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block")
    float StaminaCost = 10.0f;
};

UCLASS(BlueprintType)
class UWeaponDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Info")
    EWeaponEnums WeaponType = EWeaponEnums::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Info")
    int32 SweepTraceSocketCount = 2;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats")
    float BaseDamage = 100.0f;

    //근력 보정
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float StrengthScaling = 60.0f;
    
    //기량 보정
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Stats", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float DexterityScaling = 60.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Definitions")
    TArray<FTaggedAttackData> AttackDataArray;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Block Definitions")
    FBlockActionData BlockData;
    
    void PreloadAllMontages()
    {
        TArray<FSoftObjectPath> AssetsToLoad;
        
        for (FTaggedAttackData& TaggedData : AttackDataArray)
        {
            FAttackActionData& AttackData = TaggedData.AttackData;
            
            for (TSoftObjectPtr<UAnimMontage>& Montage : AttackData.AttackMontages)
            {
                if (!Montage.IsNull())
                {
                    AssetsToLoad.Add(Montage.ToSoftObjectPath());
                }
            }
            
            for (TSoftObjectPtr<UAnimMontage>& SubMontage : AttackData.SubAttackMontages)
            {
                if (!SubMontage.IsNull())
                {
                    AssetsToLoad.Add(SubMontage.ToSoftObjectPath());
                }
            }
        }
        
        if (!BlockData.BlockIdleMontage.IsNull())
        {
            AssetsToLoad.Add(BlockData.BlockIdleMontage.ToSoftObjectPath());
        }
        
        if (!BlockData.BlockReactionMontage.IsNull())
        {
            AssetsToLoad.Add(BlockData.BlockReactionMontage.ToSoftObjectPath());
        }
        
        //Asset Manager를 통한 로딩
        if (AssetsToLoad.Num() > 0 && UAssetManager::IsInitialized())
        {
            UAssetManager& AssetManager = UAssetManager::Get();
            FStreamableManager& StreamableManager = AssetManager.GetStreamableManager();
            
            //동기 로딩
            for (const FSoftObjectPath& AssetPath : AssetsToLoad)
            {
                StreamableManager.LoadSynchronous(AssetPath);
            }
        }
    }
};