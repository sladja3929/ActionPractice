#include "Public/Items/Weapon.h"
#include "Public/Items/WeaponData.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Math/UnrealMathUtility.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"
#include "Items/WeaponCollisionComponent.h"

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
    CollisionComponent = CreateDefaultSubobject<UWeaponCollisionComponent>(TEXT("CollisionComponent"));
    
    // 기본 콜리전 설정
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AWeapon::BeginPlay()
{
   Super::BeginPlay();
    
    // 콜리전 컴포넌트 델리게이트 바인딩
    if (CollisionComponent)
    {
        CollisionComponent->OnWeaponHit.AddUObject(this, &AWeapon::HandleWeaponHit);
        
        // WeaponData에서 기본 프로파일 설정
        if (WeaponData)
        {
            // 기본 소켓 설정 (WeaponData에 있다면)
            FWeaponTraceProfile DefaultProfile;
            
            // 무기 타입에 따른 기본 소켓 설정
            switch (WeaponType)
            {
                case WeaponEnums::Sword:
                case WeaponEnums::GreatSword:
                    DefaultProfile.SocketNames = {TEXT("WeaponStart"), TEXT("WeaponEnd")};
                    DefaultProfile.DamageType = EAttackDamageType::Slash;
                    DefaultProfile.TraceRadius = 15.0f;
                    break;
                    
                case WeaponEnums::Spear:
                    DefaultProfile.SocketNames = {TEXT("WeaponTip")};
                    DefaultProfile.DamageType = EAttackDamageType::Pierce;
                    DefaultProfile.TraceRadius = 10.0f;
                    break;
                    
                case WeaponEnums::Hammer:
                    DefaultProfile.SocketNames = {TEXT("WeaponHead")};
                    DefaultProfile.DamageType = EAttackDamageType::Blunt;
                    DefaultProfile.TraceRadius = 20.0f;
                    break;
                    
                default:
                    DefaultProfile.SocketNames = {TEXT("WeaponStart"), TEXT("WeaponEnd")};
                    DefaultProfile.DamageType = EAttackDamageType::Slash;
                    DefaultProfile.TraceRadius = 10.0f;
                    break;
            }
            
            // 소켓별 데미지 배율 설정
            if (DefaultProfile.SocketNames.Num() >= 2)
            {
                DefaultProfile.SocketDamageMultipliers.Add(DefaultProfile.SocketNames[0], 0.8f);  // 손잡이 쪽
                DefaultProfile.SocketDamageMultipliers.Add(DefaultProfile.SocketNames[1], 1.2f);  // 끝 쪽
            }
            else if (DefaultProfile.SocketNames.Num() == 1)
            {
                DefaultProfile.SocketDamageMultipliers.Add(DefaultProfile.SocketNames[0], 1.0f);
            }
            
            CollisionComponent->SetTraceProfile(DefaultProfile);
            
            DEBUG_LOG(TEXT("Weapon initialized with %d sockets, DamageType: %d"),
                     DefaultProfile.SocketNames.Num(), (int32)DefaultProfile.DamageType);
        }
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
    if (!FirstMontage.IsNull() && !FirstMontage.IsValid()) WeaponData->PreloadAllMontages();;
    
    return &WeaponData->BlockData;
}


const FAttackActionData* AWeapon::GetWeaponAttackDataByTag(FGameplayTag AttackTag) const
{
    if (!WeaponData) return nullptr;

    else
    {
        // 첫 번째 몽타주를 체크해서, 로드가 안되었으면 로드
        for (const auto& AttackDataPair : WeaponData->AttackDataMap)
        {
            if (AttackDataPair.Value.AttackMontages.Num() > 0)
            {
                const TSoftObjectPtr<UAnimMontage>& FirstMontage = AttackDataPair.Value.AttackMontages[0];
                if (!FirstMontage.IsNull() && !FirstMontage.IsValid())
                {
                    WeaponData->PreloadAllMontages();
                    break;
                }
                break;  // 첫 번째만 체크
            }
        }
    }
    
    return WeaponData->AttackDataMap.Find(AttackTag);
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