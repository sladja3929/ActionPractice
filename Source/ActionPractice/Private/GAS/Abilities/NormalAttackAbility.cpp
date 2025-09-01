#include "GAS/Abilities/NormalAttackAbility.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "Characters/InputBufferComponent.h"
#include "Items/WeaponData.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Animation/AnimMontage.h"
#include "GAS/GameplayTagsSubsystem.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Characters/ActionPracticeCharacter.h"
#include "GAS/Abilities/Tasks/AbilityTask_PlayMontageWithEvents.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
    #define DEBUG_LOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

UNormalAttackAbility::UNormalAttackAbility()
{
    StaminaCost = 15.0f;
    ComboCounter = 0;
    MaxComboCount = 1;
}

void UNormalAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

    //이벤트 태스크 실행
    WaitPlayBufferEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
        this, UGameplayTagsSubsystem::GetEventActionPlayBufferTag(), nullptr, false, true);
    WaitPlayBufferEventTask->EventReceived.AddDynamic(this, &UNormalAttackAbility::OnEventPlayBuffer);
    WaitPlayBufferEventTask->ReadyForActivation();
    
    //무기 데이터 적용
    MaxComboCount = WeaponAttackData->ComboAttackData.Num();
    
    //차지어택에 따라 추후 변경
    ComboCounter = 0;

    MontageToPlay = WeaponAttackData->AttackMontages[ComboCounter].Get();
    bCreateTask = true;
    PlayMontage();
}

void UNormalAttackAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{     
    // 3-2. ActionRecoveryEnd 이후 구간에서 입력이 들어오면 콤보 실행
    if (!GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag()))
    {
        PlayNextAttack();
        DEBUG_LOG(TEXT("Input Pressed - After Recovery"));
    }    
}


void UNormalAttackAbility::PlayNextAttack()
{
    ++ComboCounter;
    
    if (ComboCounter >= MaxComboCount)
    {
        ComboCounter = 0;
    }
    
    DEBUG_LOG(TEXT("NextAttack - ComboCounter: %d"),ComboCounter);
    MontageToPlay = WeaponAttackData->AttackMontages[ComboCounter].Get();
    bCreateTask = false;
    PlayMontage();
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

void UNormalAttackAbility::ExecuteMontageTask()
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
            PlayMontageWithEventsTask->OnMontageCompleted.AddDynamic(this, &UNormalAttackAbility::OnTaskMontageCompleted);
            PlayMontageWithEventsTask->OnMontageInterrupted.AddDynamic(this, &UNormalAttackAbility::OnTaskMontageInterrupted);
            PlayMontageWithEventsTask->OnResetCombo.AddDynamic(this, &UNormalAttackAbility::OnNotifyResetCombo);

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

void UNormalAttackAbility::OnEventPlayBuffer(FGameplayEventData Payload)
{
    PlayNextAttack();
    DEBUG_LOG(TEXT("Attack Recovery End - Play Next Attack"));
}

void UNormalAttackAbility::OnNotifyResetCombo()
{
    DEBUG_LOG(TEXT("Reset Combo"));
    ComboCounter = -1; //어빌리티가 살아있는 동안 입력이 들어오면 PlayNext로 0이 되고, 어빌리티가 죽으면 초기화
}

void UNormalAttackAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
    DEBUG_LOG(TEXT("AttackAbility Cancelled"));    
    Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UNormalAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ComboCounter = 0;
    
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}