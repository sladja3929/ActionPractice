#include "GAS/Abilities/Tasks/AbilityTask_PlayMontageWithEvents.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GAS/GameplayTagsSubsystem.h"

// 디버그 로그 활성화/비활성화 (0: 비활성화, 1: 활성화)
#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogAbilitySystemComponent, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

UAbilityTask_PlayMontageWithEvents::UAbilityTask_PlayMontageWithEvents(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Rate = 1.0f;
    bStopMontageWhenAbilityCancelled = false;
    
    MontageToPlay = nullptr;
}

UAbilityTask_PlayMontageWithEvents* UAbilityTask_PlayMontageWithEvents::CreatePlayMontageWithEventsProxy(
    UGameplayAbility* OwningAbility,
    FName TaskInstanceName,
    UAnimMontage* MontageToPlay,
    float Rate,
    FName StartSection,
    float AnimRootMotionTranslationScale)
{
    UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

    UAbilityTask_PlayMontageWithEvents* MyTask = NewAbilityTask<UAbilityTask_PlayMontageWithEvents>(OwningAbility, TaskInstanceName);
    MyTask->MontageToPlay = MontageToPlay;
    MyTask->Rate = Rate;
    MyTask->StartSectionName = StartSection;
    MyTask->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;

    return MyTask;
}

void UAbilityTask_PlayMontageWithEvents::Activate()
{
    if (Ability == nullptr)
    {
        return;
    }
    
    DEBUG_LOG(TEXT("Task Activate"));
    
    bool bPlayedMontage = false;
    
    if (AbilitySystemComponent.IsValid())
    {
        const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
        UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

        if (AnimInstance != nullptr)
        {
            // 이벤트 콜백 등록
            RegisterGameplayEventCallbacks();
            
            // 몽타주 실행
            PlayMontage();
            bPlayedMontage = true;
        }
    }

    if (!bPlayedMontage)
    {
        EndTask();
    }

    SetWaitingOnAvatar();
}

#pragma region "Montage Play Functions"
void UAbilityTask_PlayMontageWithEvents::PlayMontage()
{
    const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
    UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
    
    if (!AnimInstance)
    {
        EndTask();
        return;
    }
    
    if (!MontageToPlay)
    {
        DEBUG_LOG(TEXT("Invalid montage"));
        EndTask();
        return;
    }
    
    float PlayLength = AnimInstance->Montage_Play(MontageToPlay, Rate);
    DEBUG_LOG(TEXT("Montage Play Result: %f, Montage Name: %s"), PlayLength, MontageToPlay ? *MontageToPlay->GetName() : TEXT("NULL"));

    // 몽타주 델리게이트 바인딩
    BindMontageCallbacks();

    ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
    if (Character && (Character->GetLocalRole() == ROLE_Authority ||
        (Character->GetLocalRole() == ROLE_AutonomousProxy && 
         Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
    {
        Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
    }
}

void UAbilityTask_PlayMontageWithEvents::ChangeMontageAndPlay(UAnimMontage* NewMontage)
{
    UAnimInstance* AnimInstance = Ability->GetCurrentActorInfo()->GetAnimInstance();
    if (!NewMontage || !IsActive() || !AnimInstance)
    {
        DEBUG_LOG(TEXT("No Montage or AnimInstance or Task"));
        //return;
    }

    UnbindMontageCallbacks();

    StopPlayingMontage();

    MontageToPlay = NewMontage;
    float PlayLength = AnimInstance->Montage_Play(MontageToPlay, Rate);
    DEBUG_LOG(TEXT("Montage Play Result: %f, Montage Name: %s"), PlayLength, MontageToPlay ? *MontageToPlay->GetName() : TEXT("NULL"));
    
    if (PlayLength > 0.0f)
    {
        // 새 콜백 바인딩
        BindMontageCallbacks();
    }
}

void UAbilityTask_PlayMontageWithEvents::StopPlayingMontage()
{
    const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
    if (!ActorInfo)
    {
        return;
    }

    UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
    if (AnimInstance && MontageToPlay)
    {
        float BlendOutTime = MontageToPlay->BlendOut.GetBlendTime();
        AnimInstance->Montage_Stop(BlendOutTime, MontageToPlay);
    }
}
#pragma endregion

#pragma region "Event Calling Functions"
void UAbilityTask_PlayMontageWithEvents::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
    DEBUG_LOG(TEXT("OnMontageBlendingOut Called - Montage: %s, Interrupted: %s"), 
           Montage ? *Montage->GetName() : TEXT("None"), 
           bInterrupted ? TEXT("True") : TEXT("False"));
           
    if (Ability && Ability->GetCurrentMontage() == MontageToPlay)
    {
        if (Montage == MontageToPlay)
        {
            AbilitySystemComponent->ClearAnimatingAbility(Ability);

            ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
            if (Character && (Character->GetLocalRole() == ROLE_Authority ||
                (Character->GetLocalRole() == ROLE_AutonomousProxy && 
                 Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
            {
                Character->SetAnimRootMotionTranslationScale(1.0f);
            }
        }
    }

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnMontageBlendOut.Broadcast();
    }
}

void UAbilityTask_PlayMontageWithEvents::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    DEBUG_LOG(TEXT("OnMontageEnded Called - Montage: %s, Interrupted: %s"), 
           Montage ? *Montage->GetName() : TEXT("None"), 
           bInterrupted ? TEXT("True") : TEXT("False"));
           
    if (!bInterrupted)
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnMontageCompleted.Broadcast();
        }
    }
    else
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnMontageInterrupted.Broadcast();
        }
    }

    EndTask();
}

void UAbilityTask_PlayMontageWithEvents::HandleEnableComboInputEvent(const FGameplayEventData& Payload)
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnEnableComboInput.Broadcast();
    }
}

void UAbilityTask_PlayMontageWithEvents::HandleActionRecoveryEndEvent(const FGameplayEventData& Payload)
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnActionRecoveryEnd.Broadcast();
    }
}

void UAbilityTask_PlayMontageWithEvents::HandleResetComboEvent(const FGameplayEventData& Payload)
{    
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnResetCombo.Broadcast(); 
    }
}
#pragma endregion

#pragma region "Delegate Binding Functions"
void UAbilityTask_PlayMontageWithEvents::RegisterGameplayEventCallbacks()
{
    if (AbilitySystemComponent.IsValid() && Ability)
    {
        // EnableComboInput 이벤트 - Lambda 사용
        EnableComboInputHandle = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyEnableComboInputTag())
            .AddLambda([this](const FGameplayEventData* EventData)
            {
                if (IsValid(this) && EventData)
                {
                    HandleEnableComboInputEvent(*EventData);
                }
            });

        // ActionRecoveryEnd 이벤트 - Lambda 사용
        ActionRecoveryEndHandle = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyActionRecoveryEndTag())
            .AddLambda([this](const FGameplayEventData* EventData)
            {
                if (IsValid(this) && EventData)
                {
                    HandleActionRecoveryEndEvent(*EventData);
                }
            });

        // ResetCombo 이벤트 - Lambda 사용
        ResetComboHandle = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyResetComboTag())
            .AddLambda([this](const FGameplayEventData* EventData)
            {
                if (IsValid(this) && EventData)
                {
                    HandleResetComboEvent(*EventData);
                }
            });
    }
}

void UAbilityTask_PlayMontageWithEvents::UnregisterGameplayEventCallbacks()
{
    if (AbilitySystemComponent.IsValid())
    {
        if (EnableComboInputHandle.IsValid())
        {
            AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyEnableComboInputTag())
                .Remove(EnableComboInputHandle);
        }

        if (ActionRecoveryEndHandle.IsValid())
        {
            AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyActionRecoveryEndTag())
                .Remove(ActionRecoveryEndHandle);
        }

        if (ResetComboHandle.IsValid())
        {
            AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(UGameplayTagsSubsystem::GetEventNotifyResetComboTag())
                .Remove(ResetComboHandle);
        }
    }
}

void UAbilityTask_PlayMontageWithEvents::BindMontageCallbacks()
{
    const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
    UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
    
    if (!AnimInstance || !MontageToPlay)
    {
        return;
    }

    // 블렌드 아웃 델리게이트 바인딩
    BlendingOutDelegate = FOnMontageBlendingOutStarted::CreateUObject(this, &UAbilityTask_PlayMontageWithEvents::OnMontageBlendingOut);
    AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);
    DEBUG_LOG(TEXT("BlendingOutDelegate Bound Successfully, Montage Name: %s") ,MontageToPlay ? *MontageToPlay->GetName() : TEXT("NULL"));

    // 몽타주 종료 델리게이트 바인딩
    MontageEndedDelegate = FOnMontageEnded::CreateUObject(this, &UAbilityTask_PlayMontageWithEvents::OnMontageEnded);
    AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);
    DEBUG_LOG(TEXT("MontageEndedDelegate Bound Successfully, Montage Name: %s") ,MontageToPlay ? *MontageToPlay->GetName() : TEXT("NULL"));
}

void UAbilityTask_PlayMontageWithEvents::UnbindMontageCallbacks()
{
    const FGameplayAbilityActorInfo* ActorInfo = Ability ? Ability->GetCurrentActorInfo() : nullptr;
    if (!ActorInfo)
    {
        return;
    }

    UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
    if (AnimInstance && MontageToPlay)
    {
        if (BlendingOutDelegate.IsBound())
        {
            FOnMontageBlendingOutStarted EmptyBlendDelegate;
            AnimInstance->Montage_SetBlendingOutDelegate(EmptyBlendDelegate, MontageToPlay);
            BlendingOutDelegate.Unbind();
            DEBUG_LOG(TEXT("BlendingOutDelegate Unbound Successfully, Montage Name: %s") ,MontageToPlay ? *MontageToPlay->GetName() : TEXT("NULL"));
        }
        
        if (MontageEndedDelegate.IsBound())
        {
            FOnMontageEnded EmptyEndDelegate;
            AnimInstance->Montage_SetEndDelegate(EmptyEndDelegate, MontageToPlay);
            MontageEndedDelegate.Unbind();
            DEBUG_LOG(TEXT("MontageEndedDelegate Unbound Successfully, Montage Name: %s") ,MontageToPlay ? *MontageToPlay->GetName() : TEXT("NULL"));
        }
    }
}
#pragma endregion

void UAbilityTask_PlayMontageWithEvents::OnDestroy(bool AbilityEnded)
{
    DEBUG_LOG(TEXT("Montage With Events Task Destroyed"));

    // 몽타주 정지
    if (bStopMontageWhenAbilityCancelled)
    {
        StopPlayingMontage();
    }
    
    // 이벤트 콜백 해제
    UnregisterGameplayEventCallbacks();
    
    // 몽타주 델리게이트 해제
    UnbindMontageCallbacks();
    
    bStopMontageWhenAbilityCancelled = false;
    
    // 포인터 정리
    MontageToPlay = nullptr;

    Super::OnDestroy(AbilityEnded);
}

void UAbilityTask_PlayMontageWithEvents::ExternalCancel()
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnTaskCancelled.Broadcast();
    }

    Super::ExternalCancel();
}