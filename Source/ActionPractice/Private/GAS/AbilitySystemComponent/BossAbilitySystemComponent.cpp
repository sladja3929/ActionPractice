#include "GAS/AbilitySystemComponent/BossAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Characters/BossCharacter.h"
#include "GAS/AttributeSet/BossAttributeSet.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogBossAbilitySystemComponent, Log, All);
	#define DEBUG_LOG(Format, ...) UE_LOG(LogBossAbilitySystemComponent, Warning, Format, ##__VA_ARGS__)
#else
	#define DEBUG_LOG(Format, ...)
#endif

UBossAbilitySystemComponent::UBossAbilitySystemComponent()
{
	SetIsReplicated(true);
	SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void UBossAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		CachedCharacter = Cast<ABossCharacter>(Owner);
	}
}

void UBossAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UBossAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	CachedCharacter = Cast<ABossCharacter>(InOwnerActor);

	//외부 바인딩용 신호
	OnASCInitialized.Broadcast(this);
}

const UBossAttributeSet* UBossAbilitySystemComponent::GetBossAttributeSet() const
{
	return this->GetSet<UBossAttributeSet>();
}