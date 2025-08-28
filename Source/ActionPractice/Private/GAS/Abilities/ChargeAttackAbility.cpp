#include "GAS/Abilities/ChargeAttackAbility.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Items/Weapon.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GAS/GameplayTagsSubsystem.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogAbilitySystemComponent, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

UChargeAttackAbility::UChargeAttackAbility()
{
    StaminaCost = 15.0f;
    ComboCounter = 0;
    MaxComboCount = 1;
    bCanComboSave = false;
    bComboInputSaved = false;
    bIsInCancellableRecovery = false;
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

    if (!SetWeaponAttackDataFromActorInfo())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    //SubAttack: 차지 몽타주, Attack: 공격 실행 몽타주
    if (WeaponAttackData->SubAttackMontages.Num() != WeaponAttackData->AttackMontages.Num())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    //무기 데이터 적용
    MaxComboCount = WeaponAttackData->ComboAttackData.Num();
    
    //차지어택에 따라 추후 변경
    ComboCounter = 0;
    bComboInputSaved = false;
    bCanComboSave = false;
    bIsInCancellableRecovery = false;
    bMaxCharged = false;
    bIsCharging = false;
    bNoCharge = false;
    
    DEBUG_LOG(TEXT("Charge Ability Activated"));
    ExecuteMontageTask(WeaponAttackData->SubAttackMontages[ComboCounter].Get());
}

void UChargeAttackAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    // 2. enablecomboInput 구간에서 입력이 들어오면 저장
    if (bCanComboSave)
    {
        bNoCharge = false;
        bComboInputSaved = true;
        bCanComboSave = false;
        DEBUG_LOG(TEXT("Input Pressed - Combo Saved"));
    }
    
    // 3-2. ActionRecoveryEnd 이후 구간에서 입력이 들어오면 콤보 실행
    else if (!GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag()))
    {
        bNoCharge = false;
        PlayNextChargeMontage();
        DEBUG_LOG(TEXT("Input Pressed - After Recovery"));
    }
}

void UChargeAttackAbility::ExecuteMontageTask(UAnimMontage* MontageToPlay)
{
    // 스태미나 소모
    if (!ConsumeStamina())
    {
        DEBUG_LOG(TEXT("No Stamina"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
    {
        AbilitySystemComponent->AddLooseGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag());
    }
    
    // 커스텀 태스크 생성 - 차지 몽타주
    MontageTask = UAbilityTask_PlayMontageWithEvents::CreatePlayMontageWithEventsProxy(
        this,
        NAME_None,
        MontageToPlay,
        1.0f,
        NAME_None,
        1.0f
    );
    
    if (MontageTask)
    {        
        // 델리게이트 바인딩 - 사용하지 않는 델리게이트도 있음
        MontageTask->OnMontageCompleted.AddDynamic(this, &UChargeAttackAbility::OnTaskMontageCompleted);
        MontageTask->OnMontageInterrupted.AddDynamic(this, &UChargeAttackAbility::OnTaskMontageInterrupted);
        MontageTask->OnEnableBufferInput.AddDynamic(this, &UChargeAttackAbility::OnNotifyEnableBufferInput);
        MontageTask->OnActionRecoveryEnd.AddDynamic(this, &UChargeAttackAbility::OnNotifyActionRecoveryEnd);
        MontageTask->OnResetCombo.AddDynamic(this, &UChargeAttackAbility::OnNotifyResetCombo);
        MontageTask->OnChargeStart.AddDynamic(this, &UChargeAttackAbility::OnNotifyChargeStart);
        
        // 태스크 활성화
        MontageTask->ReadyForActivation();
    }
    
    else
    {
        DEBUG_LOG(TEXT("No Montage Task"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }
}

void UChargeAttackAbility::PlayNextChargeMontage()
{
    ComboCounter++;
    bComboInputSaved = false;
    bCanComboSave = false;
    bIsInCancellableRecovery = false;
    bMaxCharged = false;
    bIsCharging = false;
    
    if (ComboCounter >= MaxComboCount)
    {
        ComboCounter = 0;
    }

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
    {
        AbilitySystemComponent->AddLooseGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag());
    }
    
    UAnimMontage* NextMontage = WeaponAttackData->SubAttackMontages[ComboCounter].Get();
    if (NextMontage)
    {
        MontageTask->ChangeMontageAndPlay(NextMontage);
    }
    else
    {
        DEBUG_LOG(TEXT("Failed to load montage for combo (charge) %d"), ComboCounter);
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UChargeAttackAbility::OnTaskMontageCompleted()
{
    //차지 몽타주가 끝날 때 = 완전히 차지했을 때
    if (bIsCharging)
    {
        bMaxCharged = true;
        
        UAnimMontage* NextMontage = WeaponAttackData->AttackMontages[ComboCounter].Get();
        if (NextMontage)
        {
            DEBUG_LOG(TEXT("Montage Completed - Max Charge"));     
            ExecuteMontageTask(NextMontage);   
        }
        else
        {
            DEBUG_LOG(TEXT("Failed to load montage for combo (attack) %d"), ComboCounter);
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        }
        
        bIsCharging = false;
    }

    else
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UChargeAttackAbility::OnNotifyEnableBufferInput()
{
    bCanComboSave = true;
}

void UChargeAttackAbility::OnNotifyActionRecoveryEnd()
{
    bCanComboSave = false;

    // 3-1. 2~3 사이 저장한 행동이 있을 경우
    if (bComboInputSaved)
    {
        PlayNextChargeMontage();
        DEBUG_LOG(TEXT("Attack Recovery End - Play Next Charge"));
    }
    // 3-2. 저장한 행동이 없을 경우
    else
    {
        if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
        {
            // 모든 StateRecovering 태그 제거 (스택된 태그 모두 제거)
            while (AbilitySystemComponent->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag()))
            {
                AbilitySystemComponent->RemoveLooseGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag());
            }
            DEBUG_LOG(TEXT("Can ABP Interrupt Attack Montage"));
        }
 
        bIsInCancellableRecovery = true;
    }
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
        UAnimMontage* NextMontage = WeaponAttackData->AttackMontages[ComboCounter].Get();
        if (NextMontage)
        {
            DEBUG_LOG(TEXT("Charge Start Notify - Next Attack: %s"), *NextMontage->GetName()); 
            MontageTask->ChangeMontageAndPlay(NextMontage);            
        }
        else
        {
            DEBUG_LOG(TEXT("Failed to load montage for combo (attack) %d"), ComboCounter);
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        }

        bIsCharging = false;
        bNoCharge = false;
    }
}

void UChargeAttackAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    //차지를 멈췄을 때
    if (bIsCharging) //차지중이라면
    {        
        UAnimMontage* NextMontage = WeaponAttackData->AttackMontages[ComboCounter].Get();
        if (NextMontage)
        {
            DEBUG_LOG(TEXT("Input Released - Next Attack: %s"), *NextMontage->GetName());     
            MontageTask->ChangeMontageAndPlay(NextMontage);            
        }
        else
        {
            DEBUG_LOG(TEXT("Failed to load montage for combo (attack) %d"), ComboCounter);
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        }

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
    bComboInputSaved = false;
    bCanComboSave = false;
    bIsInCancellableRecovery = false;
    bMaxCharged = false;
    bIsCharging = false;
    bNoCharge = false;
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}