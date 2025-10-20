// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/HitDetection/EnemyAttackComponent.h"
#include "Characters/Enemy/EnemyCharacterBase.h"
#include "Items/AttackData.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "Components/InputComponent.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogEnemyAttackComponent, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogEnemyAttackComponent, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

UEnemyAttackComponent::UEnemyAttackComponent()
{
}

void UEnemyAttackComponent::BeginPlay()
{
	OwnerEnemy = Cast<AEnemyCharacterBase>(GetOwner());
	if (!OwnerEnemy)
	{
		DEBUG_LOG(TEXT("EnemyAttackComponent: Owner is not an Enemy!"));
		return;
	}

	Super::BeginPlay();

	//디버그용 2번 키 바인딩 (1번은 Weapon이 사용)
	if (GetWorld())
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			if (PC->InputComponent)
			{
				PC->InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &UEnemyAttackComponent::ToggleWeaponDebugTrace);
			}
		}
	}
}

AActor* UEnemyAttackComponent::GetOwnerActor() const
{
	return OwnerEnemy;
}

UAbilitySystemComponent* UEnemyAttackComponent::GetOwnerASC() const
{
	if (!OwnerEnemy) return nullptr;

	return OwnerEnemy->GetAbilitySystemComponent();
}

#pragma region "Trace Config Functions"
bool UEnemyAttackComponent::LoadTraceConfig(const FGameplayTagContainer& AttackTags, int32 ComboIndex)
{
	if (!OwnerEnemy) return false;

	// TODO: EnemyDataAsset 생성 후 아래 하드코딩 부분을 데이터 에셋에서 로드하도록 변경
	// const UEnemyDataAsset* EnemyData = OwnerEnemy->GetEnemyData();
	// if (!EnemyData) return false;
	// const FAttackActionData* AttackData = OwnerEnemy->GetEnemyAttackDataByTag(AttackTags);
	// if (!AttackData || AttackData->ComboAttackData.Num() == 0) return false;
	// ComboIndex = FMath::Clamp(ComboIndex, 0, AttackData->ComboAttackData.Num() - 1);
	// const FIndividualAttackData& AttackInfo = AttackData->ComboAttackData[ComboIndex];

	// 임시 하드코딩 - EnemyDataAsset 구현 후 삭제
	CurrentTraceConfig.AttackMotionType = EAttackDamageType::Slash;
	CurrentTraceConfig.SocketCount = 2;
	CurrentTraceConfig.TraceRadius = DefaultTraceRadius;

	CurrentAttackData.FinalDamage = 50.0f;
	CurrentAttackData.PoiseDamage = 30.0f;
	CurrentAttackData.DamageType = EAttackDamageType::Slash;

	DEBUG_LOG(TEXT("LoadTraceConfig - HARDCODED DATA (TODO: Implement EnemyDataAsset)"));

	return true;
}

void UEnemyAttackComponent::SetOwnerMesh()
{
	if (!OwnerEnemy) return;

	// Enemy의 SkeletalMeshComponent 가져오기
	OwnerMesh = OwnerEnemy->GetMesh();

	if (!OwnerMesh || !OwnerMesh->DoesSocketExist(FName(*FString::Printf(TEXT("trace_socket_0")))))
	{
		OwnerMesh = nullptr;
		DEBUG_LOG(TEXT("EnemyAttackComponent: No Enemy SkeletalMesh or trace_socket_0"));
		return;
	}

	TipSocketName = FName(*FString::Printf(TEXT("%s0"), *SocketNamePrefix.ToString()));
}
#pragma endregion

#pragma region "Hit Functions"
void UEnemyAttackComponent::AddIgnoredActors(FCollisionQueryParams& Params) const
{
	if (OwnerEnemy)
	{
		Params.AddIgnoredActor(OwnerEnemy);
		// TODO: 필요시 아군 Enemy 제외 로직 추가
	}
}
#pragma endregion