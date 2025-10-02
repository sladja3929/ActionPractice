#include "GAS/ActionPracticeAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogBaseAbilitySystemComponent, Log, All);
	#define DEBUG_LOG(Format, ...) UE_LOG(LogBaseAbilitySystemComponent, Warning, Format, ##__VA_ARGS__)
#else
	#define DEBUG_LOG(Format, ...)
#endif

UActionPracticeAbilitySystemComponent::UActionPracticeAbilitySystemComponent()
{
	// 네트워크/효과 복제 기본 설정(캐릭터 구성과 일관)
	SetIsReplicated(true);
	SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void UActionPracticeAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		CachedCharacter = Cast<AActionPracticeCharacter>(Owner);
	}
	
	if (bDebugLogEnabled)
	{
		DEBUG_LOG(TEXT("ASC BeginPlay. Owner=%s, Avatar=%s"),
			*GetNameSafe(GetOwner()), *GetNameSafe(GetAvatarActor_Direct()));
	}
}

void UActionPracticeAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bDebugLogEnabled)
	{
		DEBUG_LOG(TEXT("ASC EndPlay. Reason=%d"), static_cast<int32>(EndPlayReason));
	}

	Super::EndPlay(EndPlayReason);
}

void UActionPracticeAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	CachedCharacter = Cast<AActionPracticeCharacter>(InOwnerActor);

	if (bDebugLogEnabled)
	{
		DEBUG_LOG(TEXT("InitAbilityActorInfo. Owner=%s, Avatar=%s"), *GetNameSafe(InOwnerActor), *GetNameSafe(InAvatarActor));
	}

	//외부 바인딩용 신호
	OnASCInitialized.Broadcast(this);
}

const UActionPracticeAttributeSet* UActionPracticeAbilitySystemComponent::GetActionPracticeAttributeSet() const 
{
	return this->GetSet<UActionPracticeAttributeSet>();
}