#include "GAS/Abilities/ChargeAttackAbility.h"
#include "Characters/InputBufferComponent.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Characters/ActionPracticeCharacter.h"
#include "Items/WeaponData.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GAS/GameplayTagsSubsystem.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GAS/Abilities/BaseAttackAbility.h"
#include "GAS/Abilities/WeaponAbilityStatics.h"
#include "GAS/Abilities/Tasks/AbilityTask_PlayMontageWithEvents.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

UChargeAttackAbility::UChargeAttackAbility()
{
    StaminaCost = 15.0f;
    ComboCounter = 0;
    MaxComboCount = 1;
    bMaxCharged = false;
    bIsCharging = false;
    bNoCharge = false;
}

void UChargeAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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
        DEBUG_LOG(TEXT("Cannot Load Charge Attack Data"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    //SubAttack: 차지 몽타주, Attack: 공격 실행 몽타주
    if (WeaponAttackData->SubAttackMontages.Num() != WeaponAttackData->AttackMontages.Num())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    //이벤트 태스크 실행
    WaitPlayBufferEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, UGameplayTagsSubsystem::GetEventActionPlayBufferTag(), nullptr, false, true);
    WaitPlayBufferEventTask->EventReceived.AddDynamic(this, &UChargeAttackAbility::OnEventPlayBuffer);
    WaitPlayBufferEventTask->ReadyForActivation();
    
    //무기 데이터 적용
    MaxComboCount = WeaponAttackData->ComboAttackData.Num();
    
    ComboCounter = 0;
    bMaxCharged = false;
    bIsCharging = false;
    bNoCharge = GetInputBufferComponentFromActorInfo()->bBufferActionReleased;
    
    DEBUG_LOG(TEXT("Charge Ability Activated"));
    MontageToPlay = WeaponAttackData->SubAttackMontages[ComboCounter].Get();
    bCreateTask = true;
    bIsAttackMontage = false;
    PlayAction();
}

void UChargeAttackAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{    
    // 3-2. ActionRecoveryEnd 이후 구간에서 입력이 들어오면 콤보 실행
    if (!GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag()))
    {
        bNoCharge = false;
        PlayNextCharge();
        DEBUG_LOG(TEXT("Input Pressed - After Recovery"));
    }    
}

void UChargeAttackAbility::PlayAction()
{    
    // 스태미나 소모
    if (!ConsumeStamina())
    {
        DEBUG_LOG(TEXT("No Stamina"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }
    
    //태그 부착
    if (UInputBufferComponent* IBC = GetInputBufferComponentFromActorInfo())
    {
        FGameplayEventData EventData;
        IBC->OnActionRecoveryStart(EventData);
    }
    
    //캐릭터 회전 (차지 x, 공격 o)
    if (bIsAttackMontage)
    {
        if (AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo())
        {
            Character->RotateCharacterToInputDirection(RotateTime);
        }
    }
    
    WaitDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, RotateTime);
    if (WaitDelayTask)
    {
        WaitDelayTask->OnFinish.AddDynamic(this, &UChargeAttackAbility::ExecuteMontageTask);
        WaitDelayTask->ReadyForActivation();
    }
}

void UChargeAttackAbility::PlayNextCharge()
{
    ComboCounter++;
    bMaxCharged = false;
    bIsCharging = false;
    
    if (ComboCounter >= MaxComboCount)
    {
        ComboCounter = 0;
    }

    MontageToPlay = WeaponAttackData->SubAttackMontages[ComboCounter].Get();
    bCreateTask = false;
    bIsAttackMontage = false;
    PlayAction();
}

void UChargeAttackAbility::ExecuteMontageTask()
{
    if (!MontageToPlay)
    {
        DEBUG_LOG(TEXT("No Montage to Play"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }
    
    if (bCreateTask) // 커스텀 태스크 생성
    {        
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
            // 델리게이트 바인딩 - 사용하지 않는 델리게이트도 있음
            PlayMontageWithEventsTask->OnMontageCompleted.AddDynamic(this, &UChargeAttackAbility::OnTaskMontageCompleted);
            PlayMontageWithEventsTask->OnMontageInterrupted.AddDynamic(this, &UChargeAttackAbility::OnTaskMontageInterrupted);
            PlayMontageWithEventsTask->OnResetCombo.AddDynamic(this, &UChargeAttackAbility::OnNotifyResetCombo);
            PlayMontageWithEventsTask->OnChargeStart.AddDynamic(this, &UChargeAttackAbility::OnNotifyChargeStart);

            // 태스크 활성화
            PlayMontageWithEventsTask->ReadyForActivation();
        }
    
        else
        {
            DEBUG_LOG(TEXT("No Montage Task"));
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        }
    }

    else //태스크 중간에 몽타주 바꾸기
    {
        PlayMontageWithEventsTask->ChangeMontageAndPlay(MontageToPlay);
    }
}

void UChargeAttackAbility::OnTaskMontageCompleted()
{
    //차지 몽타주가 끝날 때 = 완전히 차지했을 때
    if (bIsCharging)
    {
        bMaxCharged = true;
        
        DEBUG_LOG(TEXT("Montage Completed - Max Charge"));
        MontageToPlay = WeaponAttackData->AttackMontages[ComboCounter].Get();
        bCreateTask = true;
        bIsAttackMontage = true;
        PlayAction();  
        
        bIsCharging = false;
    }

    else
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UChargeAttackAbility::OnEventPlayBuffer(FGameplayEventData Payload)
{
    bNoCharge = GetInputBufferComponentFromActorInfo()->bBufferActionReleased;
    PlayNextCharge();
    DEBUG_LOG(TEXT("Attack Recovery End - Play Next Charge"));
}

void UChargeAttackAbility::OnNotifyResetCombo()
{
    DEBUG_LOG(TEXT("Reset Combo"));
    ComboCounter = -1; //어빌리티가 살아있는 동안 입력이 들어오면 PlayNext로 0이 되고, 어빌리티가 죽으면 초기화
}

void UChargeAttackAbility::OnNotifyChargeStart()
{
    bIsCharging = true;
      
    if (bNoCharge) //이미 뗴져 있다면 바로 공격
    {
        MontageToPlay = WeaponAttackData->AttackMontages[ComboCounter].Get();
        bCreateTask = false;
        bIsAttackMontage = true;
        PlayAction();

        bIsCharging = false;
        bNoCharge = false;
    }
}

void UChargeAttackAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    //차지를 멈췄을 때
    if (bIsCharging) //차지중이라면
    {
        MontageToPlay = WeaponAttackData->AttackMontages[ComboCounter].Get();
        bCreateTask = false;
        bIsAttackMontage = true;
        PlayAction();

        bIsCharging = false;
    }

    else //차지중이 아니라면 (선딜 전에 뗌)
    {
        DEBUG_LOG(TEXT("Input Released - No Charge true"));    
        bNoCharge = true;
    }
}

void UChargeAttackAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
    DEBUG_LOG(TEXT("AttackAbility Cancelled"));    
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UChargeAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ComboCounter = 0;
    bMaxCharged = false;
    bIsCharging = false;
    bNoCharge = false;
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}