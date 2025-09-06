#include "GAS/Abilities/BaseAttackAbility.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Items/WeaponData.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Characters/ActionPracticeCharacter.h"
#include "Characters/HitDetectionInterface.h"
#include "GAS/Abilities/WeaponAbilityStatics.h"
#include "GAS/Abilities/Tasks/AbilityTask_PlayMontageWithEvents.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

UBaseAttackAbility::UBaseAttackAbility()
{
    StaminaCost = 15.0f;
    RotateTime = 0.1f;
    WeaponAttackData = nullptr;
    PlayMontageWithEventsTask = nullptr;
    WaitPlayBufferEventTask = nullptr;
}

void UBaseAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        DEBUG_LOG(TEXT("Cannot Commit Ability"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    WeaponAttackData = FWeaponAbilityStatics::GetAttackDataFromAbility(this);
    if (!WeaponAttackData)
    {
        DEBUG_LOG(TEXT("Cannot Load Base Attack Data"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    MontageToPlay = WeaponAttackData->AttackMontages[0].Get();
    
    PlayAction();
}


void UBaseAttackAbility::PlayAction()
{
    if (AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo())
    {
        if (TScriptInterface<IHitDetectionInterface> HitDetection = Character->GetHitDetectionInterface())
        {
            //어빌리티 태그를 HitDetection에 제공
            FGameplayTag MainTag = AbilityTags.First();
            if (MainTag.IsValid()) HitDetection->PrepareHitDetection(MainTag);
        }
    }
    
    
    Super::PlayAction();
}

void UBaseAttackAbility::ExecuteMontageTask()
{
    if (!MontageToPlay)
    {
        DEBUG_LOG(TEXT("No Montage to Play"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }
    
    // 커스텀 태스크 생성
    PlayMontageWithEventsTask = UAbilityTask_PlayMontageWithEvents::CreatePlayMontageWithEventsProxy(
        this,
        NAME_None,
        MontageToPlay,
        1.0f,
        NAME_None,
        1.0f
    );
    
    if (PlayMontageWithEventsTask)
    {        
        // 델리게이트 바인딩 - 부모 클래스의 함수 사용
        PlayMontageWithEventsTask->OnMontageCompleted.AddDynamic(this, &UBaseAttackAbility::OnTaskMontageCompleted);
        PlayMontageWithEventsTask->OnMontageInterrupted.AddDynamic(this, &UBaseAttackAbility::OnTaskMontageInterrupted);

        // 태스크 활성화
        PlayMontageWithEventsTask->ReadyForActivation();
    }
    
    else
    {
        DEBUG_LOG(TEXT("No Montage Task"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }
}     

void UBaseAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    DEBUG_LOG(TEXT("EndAbility %d"), bWasCancelled);
    
    if (IsEndAbilityValid(Handle, ActorInfo))
    {
        if (PlayMontageWithEventsTask)
        {
            PlayMontageWithEventsTask->bStopMontageWhenAbilityCancelled = bWasCancelled;
        }

        Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }
}
