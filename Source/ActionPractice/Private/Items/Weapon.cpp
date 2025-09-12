#include "Public/Items/Weapon.h"
#include "Public/Items/WeaponDataAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Math/UnrealMathUtility.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"
#include "Characters/ActionPracticeCharacter.h"
#include "Items/WeaponAttackTraceComponent.h"
#include "Items/WeaponCCDComponent.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = true;

    // 메시 컴포넌트 생성
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;
    
    // 콜리전 컴포넌트 추가
    AttackTraceComponent = CreateDefaultSubobject<UWeaponAttackTraceComponent>(TEXT("TraceComponent"));
    CCDComponent = CreateDefaultSubobject<UWeaponCCDComponent>(TEXT("CCDComponent"));
    
    // 기본 콜리전 설정
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AWeapon::BeginPlay()
{
    OwnerCharacter = Cast<AActionPracticeCharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        DEBUG_LOG(TEXT("No Owner Character In Weapon"));
        return;
    }

    Super::BeginPlay();

    // 콜리전 컴포넌트 델리게이트 바인딩
    if (AttackTraceComponent)
    {
        AttackTraceComponent->OnWeaponHit.AddUObject(this, &AWeapon::HandleWeaponHit);
    }

    if (CCDComponent)
    {
        CCDComponent->OnWeaponHit.AddUObject(this, &AWeapon::HandleWeaponHit);
    }

}


void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

EWeaponEnums AWeapon::GetWeaponType() const
{
    if (!WeaponData) return EWeaponEnums::None;

    return WeaponData->WeaponType;    
}

const FBlockActionData* AWeapon::GetWeaponBlockData() const
{
    if (!WeaponData) return nullptr;

    // 첫 번째 몽타주를 체크해서, 로드가 안되었으면 로드
    const TSoftObjectPtr<UAnimMontage>& FirstMontage = WeaponData->BlockData.BlockIdleMontage;
    if (!FirstMontage.IsNull() && !FirstMontage.IsValid()) WeaponData->PreloadAllMontages();
    
    return &WeaponData->BlockData;
}


const FAttackActionData* AWeapon::GetWeaponAttackDataByTag(const FGameplayTagContainer& AttackTags) const
{
    if (!WeaponData) return nullptr;

    // 첫 번째 몽타주를 체크해서, 로드가 안되었으면 로드
    for (const FTaggedAttackData& TaggedData : WeaponData->AttackDataArray)
    {
        if (TaggedData.AttackData.AttackMontages.Num() > 0)
        {
            const TSoftObjectPtr<UAnimMontage>& FirstMontage = TaggedData.AttackData.AttackMontages[0];
            if (!FirstMontage.IsNull() && !FirstMontage.IsValid())
            {
                WeaponData->PreloadAllMontages();
                break;
            }
        }
    }
    
    // 정확한 매칭: 전달받은 태그 컨테이너와 정확히 일치하는 키를 찾음
    for (const FTaggedAttackData& TaggedData : WeaponData->AttackDataArray)
    {
        if (TaggedData.AttackTags == AttackTags)
        {
            return &TaggedData.AttackData;
        }
    }
    
    return nullptr;
}

TScriptInterface<IHitDetectionInterface> AWeapon::GetHitDetectionComponent() const
{
    if (bIsTraceDetectionOrNot) return AttackTraceComponent;
    return CCDComponent;
}


void AWeapon::EquipWeapon()
{    
    // 여기에 무기별 고유 로직을 추가할 수 있습니다
    // 예: 몽타주 재생, 이펙트 재생, 사운드 재생, 데미지 처리 등
}


void AWeapon::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (OtherActor && OtherActor != this)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon %s hit %s"), *WeaponName, *OtherActor->GetName());
        // 여기에 히트 처리 로직 추가
        // 예: 데미지 처리, 이펙트 재생 등
    }
}

void AWeapon::HandleWeaponHit(AActor* HitActor, const FHitResult& HitResult, EAttackDamageType DamageType, float DamageMultiplier)
{
    
}
