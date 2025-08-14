#include "GAS/Abilities/Tasks/AbilityTask_PlayNormalAttackMontage.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UAbilityTask_PlayNormalAttackMontage::UAbilityTask_PlayNormalAttackMontage(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Rate = 1.0f;
    bStopMontageWhenAbilityCancelled = false;
    
    ComboCounter = 0;
    MaxComboCount = 3;
    bCanComboSave = false;
    bComboInputSaved = false;
    bIsInCancellableRecovery = false;
}

UAbilityTask_PlayNormalAttackMontage* UAbilityTask_PlayNormalAttackMontage::CreatePlayNormalAttackMontageProxy(
    UGameplayAbility* OwningAbility,
    FName TaskInstanceName,
    UAnimMontage* MontageToPlay,
    float Rate,
    FName StartSection,
    float AnimRootMotionTranslationScale)
{
    UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

    UAbilityTask_PlayNormalAttackMontage* MyTask = NewAbilityTask<UAbilityTask_PlayNormalAttackMontage>(OwningAbility, TaskInstanceName);
    MyTask->MontageToPlay = MontageToPlay;
    MyTask->Rate = Rate;
    MyTask->StartSectionName = StartSection;
    MyTask->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;

    return MyTask;
}

void UAbilityTask_PlayNormalAttackMontage::Activate()
{
    if (Ability == nullptr)
    {
        return;
    }

    bool bPlayedMontage = false;

    if (AbilitySystemComponent.IsValid())
    {
        const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
        UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

        if (AnimInstance != nullptr)
        {
            // 이벤트 콜백 등록
            RegisterGameplayEventCallbacks();
            
            // 첫 공격 실행
            PlayAttackMontage();
            bPlayedMontage = true;
        }
    }

    if (!bPlayedMontage)
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnCancelled.Broadcast();
        }
        EndTask();
    }

    SetWaitingOnAvatar();
}

/* 공격 수행 메커니즘
 * 1. 몽타주 실행 (State.IsAttacking 태그 추가)
 * 2. enablecomboInput = 입력 저장 가능 구간, 다음 공격과 구르기 저장 가능 (구르기를 저장해도 다음 공격 우선 저장)
 * 3. AttackRecoveryEnd = 공격 선딜이 끝나는 지점
 * 3-1. 2~3 사이 저장한 행동이 있을 경우 CheckComboInput으로 행동 수행
 * 3-2. 2~3 사이 저장한 행동이 없을 경우 입력이 들어오면 다음 공격 가능, 이동/점프/구르기로 캔슬 가능 (State.IsAttacking 태그 제거)
 * 4. ResetCombo = 공격 콤보가 초기화되어 다음 콤보로 연계되지 않음, 이후 바로 어빌리티 종료
 * 5. 몽타주 종료 (ResetCombo와 같지 않음)
 */
//AttackRecoveryEnd -> RecoveryEnd
//State.Recovering
#pragma region "Attack Functions"
void UAbilityTask_PlayNormalAttackMontage::PlayAttackMontage()
{
    const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
    UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
    
    if (!AnimInstance || !MontageToPlay)
    {
        EndTask();
        return;
    }

    bComboInputSaved = false;
    bCanComboSave = false;
    bIsInCancellableRecovery = false;

    if (AbilitySystemComponent.IsValid())
    {
        AbilitySystemComponent->AddLooseGameplayTag(AttackStateTag);
    }
    
    AnimInstance->Montage_Play(MontageToPlay, Rate);

    // 블렌드 아웃 델리게이트
    BlendingOutDelegate = FOnMontageBlendingOutStarted::CreateUObject(this, &UAbilityTask_PlayNormalAttackMontage::OnMontageBlendingOut);
    AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

    // 몽타주 종료 델리게이트
    MontageEndedDelegate = FOnMontageEnded::CreateUObject(this, &UAbilityTask_PlayNormalAttackMontage::OnMontageEnded);
    AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

    ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
    if (Character && (Character->GetLocalRole() == ROLE_Authority ||
        (Character->GetLocalRole() == ROLE_AutonomousProxy && 
         Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
    {
        Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
    }

    // 콤보 실행 알림
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnComboPerformed.Broadcast();
    }
    UE_LOG(LogAbilitySystemComponent, Warning, TEXT("Attack Monatage First Played"));
}

void UAbilityTask_PlayNormalAttackMontage::JumpToNextAttackSection()
{
    bComboInputSaved = false;
    bCanComboSave = false;
    bIsInCancellableRecovery = false;
    ComboCounter++;
    
    if (ComboCounter >= MaxComboCount)
    {
        EndTask();
        return;
    }
    
    const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
    UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
    
    if (!AnimInstance || !MontageToPlay)
    {
        EndTask();
        return;
    }

    if (AbilitySystemComponent.IsValid())
    {
        AbilitySystemComponent->AddLooseGameplayTag(AttackStateTag);
    }

    FName SectionName = FName(*FString::Printf(TEXT("Attack%d"), ComboCounter + 1));
    AnimInstance->Montage_JumpToSection(SectionName, MontageToPlay);

    // 콤보 실행 알림
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnComboPerformed.Broadcast();
    }
    UE_LOG(LogAbilitySystemComponent, Warning, TEXT("Attack Monatage Combo Played"));
}

void UAbilityTask_PlayNormalAttackMontage::CheckComboInputPreseed() //어빌리티 실행 중 입력이 들어올 때
{
    // 2. enablecomboInput 구간에서 입력이 들어오면 저장
    if (bCanComboSave)
    {
        bComboInputSaved = true;
        bCanComboSave = false;
        UE_LOG(LogAbilitySystemComponent, Warning, TEXT("Combo Saved"));
    }
    // 3-2. AttackRecoveryEnd 이후 구간에서 입력이 들어오면 콤보 실행
    else if (bIsInCancellableRecovery)
    {
        UE_LOG(LogAbilitySystemComponent, Warning, TEXT("Combo Played After Recovery"));
        JumpToNextAttackSection();
    }
}

#pragma endregion

#pragma region "AnimNotify Event Functions"
void UAbilityTask_PlayNormalAttackMontage::HandleEnableComboInputEvent(const FGameplayEventData& Payload)
{
    bCanComboSave = true;
}

void UAbilityTask_PlayNormalAttackMontage::HandleAttackRecoveryEndEvent(const FGameplayEventData& Payload)
{
    bCanComboSave = false;

    // 3-1. 2~3 사이 저장한 행동이 있을 경우
    if (bComboInputSaved)
    {
        UE_LOG(LogAbilitySystemComponent, Warning, TEXT("Combo Played For Saved"));
        JumpToNextAttackSection();
    }
    // 3-2. 저장한 행동이 없을 경우
    else
    {
        if (AbilitySystemComponent.IsValid())
        {
            AbilitySystemComponent->RemoveLooseGameplayTag(AttackStateTag);
        }
 
        bIsInCancellableRecovery = true;
    }
}

// 4. ResetCombo 이벤트 수신
void UAbilityTask_PlayNormalAttackMontage::HandleResetComboEvent(const FGameplayEventData& Payload)
{
    EndTask();
}

void UAbilityTask_PlayNormalAttackMontage::RegisterGameplayEventCallbacks()
{
    if (AbilitySystemComponent.IsValid() && Ability)
    {
        // EnableComboInput 이벤트 - Lambda 사용
        EnableComboInputHandle = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(EventTag_EnableComboInput)
            .AddLambda([this](const FGameplayEventData* EventData)
            {
                if (IsValid(this) && EventData)
                {
                    HandleEnableComboInputEvent(*EventData);
                }
            });

        // AttackRecoveryEnd 이벤트 - Lambda 사용
        AttackRecoveryEndHandle = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(EventTag_AttackRecoveryEnd)
            .AddLambda([this](const FGameplayEventData* EventData)
            {
                if (IsValid(this) && EventData)
                {
                    HandleAttackRecoveryEndEvent(*EventData);
                }
            });

        // ResetCombo 이벤트 - Lambda 사용
        ResetComboHandle = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(EventTag_ResetCombo)
            .AddLambda([this](const FGameplayEventData* EventData)
            {
                if (IsValid(this) && EventData)
                {
                    HandleResetComboEvent(*EventData);
                }
            });
    }
}

void UAbilityTask_PlayNormalAttackMontage::UnregisterGameplayEventCallbacks()
{
    if (AbilitySystemComponent.IsValid())
    {
        if (EnableComboInputHandle.IsValid())
        {
            AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(EventTag_EnableComboInput)
                .Remove(EnableComboInputHandle);
        }

        if (AttackRecoveryEndHandle.IsValid())
        {
            AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(EventTag_AttackRecoveryEnd)
                .Remove(AttackRecoveryEndHandle);
        }

        if (ResetComboHandle.IsValid())
        {
            AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(EventTag_ResetCombo)
                .Remove(ResetComboHandle);
        }
    }
}
#pragma endregion

#pragma region "Montage Functions"
void UAbilityTask_PlayNormalAttackMontage::StopPlayingMontage()
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

void UAbilityTask_PlayNormalAttackMontage::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
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

    if (bInterrupted)
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnInterrupted.Broadcast();
        }
    }
    else
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnBlendOut.Broadcast();
        }
    }
}

void UAbilityTask_PlayNormalAttackMontage::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bInterrupted)
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnCompleted.Broadcast();
        }
    }
    else
    {
        if (ShouldBroadcastAbilityTaskDelegates())
        {
            OnInterrupted.Broadcast();
        }
    }

    EndTask();
}

void UAbilityTask_PlayNormalAttackMontage::ExternalCancel()
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        OnCancelled.Broadcast();
    }

    Super::ExternalCancel();
}
#pragma endregion

void UAbilityTask_PlayNormalAttackMontage::OnDestroy(bool AbilityEnded)
{
    // 이벤트 콜백 해제
    UnregisterGameplayEventCallbacks();

    // 상태 정리
    if (AbilitySystemComponent.IsValid())
    {
        AbilitySystemComponent->RemoveLooseGameplayTag(AttackStateTag);
    }
    
    bCanComboSave = false;
    bComboInputSaved = false;
    bIsInCancellableRecovery = false;

    // 몽타주 정지
    if (bStopMontageWhenAbilityCancelled)
    {
        StopPlayingMontage();
    }

    Super::OnDestroy(AbilityEnded);
}