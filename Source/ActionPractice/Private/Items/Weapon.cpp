#include "Public/Items/Weapon.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Math/UnrealMathUtility.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = true;

    // Static Mesh Component 생성 및 RootComponent로 설정
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    // 콜리전 설정 - 메시 모양 그대로 사용
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    WeaponMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    
    // 메시의 실제 형태를 콜리전으로 사용 (Complex Collision)
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
    WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
    
    // Complex Collision 사용 설정 (메시 모양 그대로)
    WeaponMesh->SetGenerateOverlapEvents(true);
    WeaponMesh->SetNotifyRigidBodyCollision(true);
    
    // Hit 이벤트 바인딩
    WeaponMesh->OnComponentHit.AddDynamic(this, &AWeapon::OnHit);
    
    WeaponName = TEXT("DefaultWeapon");
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();
    
    // 런타임에서 Complex Collision 설정 강제 적용
    if (WeaponMesh && WeaponMesh->GetStaticMesh())
    {
        // 메시의 실제 형태를 Simple Collision으로도 사용
        //WeaponMesh->GetStaticMesh()->GetBodySetup()->CollisionTraceFlag = ECollisionTraceFlag::CTF_UseComplexAsSimple;
        
        // 콜리전 업데이트
        WeaponMesh->UpdateCollisionFromStaticMesh();
    }
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

const FBlockActionData* AWeapon::GetWeaponBlockData() const
{
    if (!WeaponData) return nullptr;

    return &WeaponData->BlockData;
}


const FAttackActionData* AWeapon::GetWeaponAttackDataByTag(FGameplayTag AttackTag) const
{
    if (!WeaponData) return nullptr;
    
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