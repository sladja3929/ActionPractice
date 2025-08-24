#include "GAS/Abilities/AttackAbility.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Items/Weapon.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GameplayTagContainer.h"
#include "GAS/GameplayTagsSubsystem.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogAbilitySystemComponent, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

UAttackAbility::UAttackAbility()
{
    StaminaCost = 15.0f;
    ComboCounter = 0;
    MaxComboCount = 1;
    bCanComboSave = false;
    bComboInputSaved = false;
    bIsInCancellableRecovery = false;
}

void UAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        DEBUG_LOG(TEXT("Cannot Commit Ability"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 무기 정보 가져오기
    AActionPracticeCharacter* Character = GetActionPracticeCharacterFromActorInfo();
    if (!Character)
    {
        DEBUG_LOG(TEXT("No Character"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    AWeapon* Weapon = Character->GetRightWeapon();
    if (!Weapon)
    {
        DEBUG_LOG(TEXT("No Weapon"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    FGameplayTag MainTag = this->AbilityTags.First();
    if (!MainTag.IsValid())
    {
        DEBUG_LOG(TEXT("No Ability MainTag"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    WeaponAttackData = Weapon->GetWeaponAttackDataByTag(MainTag);
    if (WeaponAttackData->AttackMontages.IsEmpty() || !WeaponAttackData->AttackMontages[0])
    {
        DEBUG_LOG(TEXT("No Montage Data - ListEmpty: %d, FirstEmpty: %d"), WeaponAttackData->AttackMontages.IsEmpty(), !WeaponAttackData->AttackMontages[0]);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
    
    // 몽타주 배열과 콤보 데이터 배열 크기 일치 검증
    if (WeaponAttackData->AttackMontages.Num() != WeaponAttackData->ComboAttackData.Num())
    {
        UE_LOG(LogAbilitySystemComponent, Error, TEXT("AttackMontages and ComboAttackData array size mismatch!"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }

    //무기 데이터 적용
    MaxComboCount = WeaponAttackData->ComboAttackData.Num();
    
    //차지어택에 따라 추후 변경
    ComboCounter = 0;
    bComboInputSaved = false;
    bCanComboSave = false;
    bIsInCancellableRecovery = false;
    
    DEBUG_LOG(TEXT("Starting Attack Montage"));
    ExecuteMontageTask();
}

void UAttackAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    // 2. enablecomboInput 구간에서 입력이 들어오면 저장
    if (bCanComboSave)
    {
        bComboInputSaved = true;
        bCanComboSave = false;
        DEBUG_LOG(TEXT("Combo Saved"));
    }
    
    // 3-2. ActionRecoveryEnd 이후 구간에서 입력이 들어오면 콤보 실행
    else if (bIsInCancellableRecovery)
    {
        PlayNextAttackCombo();
        DEBUG_LOG(TEXT("Combo Played After Recovery"));
    }
}

/* 공격 수행 메커니즘
 * 1. 몽타주 실행 (State.IsRecovering 태그 추가)
 * 2. enablecomboInput = 입력 저장 가능 구간, 다음 공격과 구르기 저장 가능 (구르기를 저장해도 다음 공격 우선 저장)
 * 3. ActionRecoveryEnd = 공격 선딜이 끝나는 지점
 * 3-1. 2~3 사이 저장한 행동이 있을 경우 CheckComboInput으로 행동 수행
 * 3-2. 2~3 사이 저장한 행동이 없을 경우 입력이 들어오면 다음 공격 가능, 이동/점프/구르기로 캔슬 가능 (State.IsRecovering 태그 제거)
 * 4. ResetCombo = 공격 콤보가 초기화되어 다음 콤보로 연계되지 않음
 * 5. 몽타주 종료 (ResetCombo와 같지 않음)
 */

void UAttackAbility::ExecuteMontageTask()
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
    
    // 커스텀 태스크 생성 - 몽타주 배열 전달
    MontageTask = UAbilityTask_PlayMontageWithEvents::CreatePlayMontageWithEventsProxy(
        this,
        NAME_None,
        WeaponAttackData->AttackMontages[ComboCounter].Get(),
        1.0f,
        NAME_None,
        1.0f
    );
    
    if (MontageTask)
    {        
        // 델리게이트 바인딩 - 사용하지 않는 델리게이트도 있음
        MontageTask->OnMontageCompleted.AddDynamic(this, &UAttackAbility::OnTaskMontageCompleted);
        MontageTask->OnMontageInterrupted.AddDynamic(this, &UAttackAbility::OnTaskMontageInterrupted);
        MontageTask->OnEnableComboInput.AddDynamic(this, &UAttackAbility::OnNotifyEnableComboInput);
        MontageTask->OnActionRecoveryEnd.AddDynamic(this, &UAttackAbility::OnNotifyActionRecoveryEnd);
        MontageTask->OnResetCombo.AddDynamic(this, &UAttackAbility::OnNotifyResetCombo);

        // 태스크 활성화
        MontageTask->ReadyForActivation();
    }
    
    else
    {
        DEBUG_LOG(TEXT("No Montage Task"));
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }
}

void UAttackAbility::PlayNextAttackCombo()
{
    ComboCounter++;
    bComboInputSaved = false;
    bCanComboSave = false;
    bIsInCancellableRecovery = false;
    
    if (ComboCounter >= MaxComboCount)
    {
        ComboCounter = 0;
    }

    if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
    {
        AbilitySystemComponent->AddLooseGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag());
    }
    
    DEBUG_LOG(TEXT("Next Combo: %d"), ComboCounter);
    
    UAnimMontage* NextMontage = WeaponAttackData->AttackMontages[ComboCounter].Get();
    if (NextMontage)
    {
        MontageTask->ChangeMontageAndPlay(NextMontage);
    }
    else
    {
        // 프리로드가 실패했거나 누락된 경우 폴백으로 동기 로딩 시도
        DEBUG_LOG(TEXT("Montage not preloaded, attempting LoadSynchronous for combo %d"), ComboCounter);
        NextMontage = WeaponAttackData->AttackMontages[ComboCounter].LoadSynchronous();
        
        if (NextMontage)
        {
            MontageTask->ChangeMontageAndPlay(NextMontage);
        }
        else
        {
            DEBUG_LOG(TEXT("Failed to load montage for combo %d"), ComboCounter);
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        }
    }
}

void UAttackAbility::OnTaskMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    DEBUG_LOG(TEXT("Task Completed - EndAbility"));
}

void UAttackAbility::OnTaskMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    DEBUG_LOG(TEXT("Task Interrupted - EndAbility"));
}

void UAttackAbility::OnNotifyEnableComboInput()
{
    bCanComboSave = true;
}

void UAttackAbility::OnNotifyActionRecoveryEnd()
{
    bCanComboSave = false;

    // 3-1. 2~3 사이 저장한 행동이 있을 경우
    if (bComboInputSaved)
    {
        PlayNextAttackCombo();
        DEBUG_LOG(TEXT("Combo Played With Saved"));
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

void UAttackAbility::OnNotifyResetCombo()
{
    ComboCounter = 0;
}

void UAttackAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
    DEBUG_LOG(TEXT("AttackAbility Cancelled"));    
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    DEBUG_LOG(TEXT("EndAbility %d"), bWasCancelled);

    ComboCounter = 0;
    bComboInputSaved = false;
    bCanComboSave = false;
    bIsInCancellableRecovery = false;
    
    if (IsEndAbilityValid(Handle, ActorInfo))
    {
        if (MontageTask)
        {
            MontageTask->bStopMontageWhenAbilityCancelled = bWasCancelled;
        }

        Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    }
}