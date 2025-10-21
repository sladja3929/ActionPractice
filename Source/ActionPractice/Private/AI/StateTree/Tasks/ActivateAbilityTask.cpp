#include "AI/StateTree/Tasks/ActivateAbilityTask.h"
#include "AbilitySystemComponent.h"
#include "StateTreeExecutionContext.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
    DEFINE_LOG_CATEGORY_STATIC(LogActivateAbilityTask, Log, All);
    #define DEBUG_LOG(Format, ...) UE_LOG(LogActivateAbilityTask, Warning, Format, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(Format, ...)
#endif

EStateTreeRunStatus FActivateAbilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    if (Transition.ChangeType != EStateTreeStateChangeType::Changed)
    {
        return EStateTreeRunStatus::Failed;
    }
    
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    //초기화
    InstanceData.bAbilityEnded = false;
    InstanceData.bAbilityCancelled = false;

    if (!InstanceData.AbilitySystemComponent)
    {
        DEBUG_LOG(TEXT("AbilitySystemComponent is nullptr"));
        return EStateTreeRunStatus::Failed;
    }

    if (!InstanceData.AbilityToActivate)
    {
        DEBUG_LOG(TEXT("AbilityToActivate is nullptr"));
        return EStateTreeRunStatus::Failed;
    }
    
    FGameplayAbilitySpec* Spec = InstanceData.AbilitySystemComponent->FindAbilitySpecFromClass(InstanceData.AbilityToActivate);

    if (!Spec)
    {
        DEBUG_LOG(TEXT("No AbilitySpec: %s"), *GetNameSafe(InstanceData.AbilityToActivate));
    }
    
    InstanceData.AbilityHandle = Spec->Handle;
    DEBUG_LOG(TEXT("Using Ability: %s"), *InstanceData.AbilityToActivate->GetName());

    //Task는 UStruct라서 어빌리티 종료 바인딩을 함수로 사용하지 못함, 람다 함수를 통해 플래그만 받고 Tick에서 확인
    InstanceData.AbilityEndedHandle = InstanceData.AbilitySystemComponent->OnAbilityEnded.AddLambda([&InstanceData](const FAbilityEndedData& EndData) {
        InstanceData.bAbilityEnded = true;
        InstanceData.bAbilityCancelled = EndData.bWasCancelled;
        if (EndData.bWasCancelled)
        {
            DEBUG_LOG(TEXT("Ability cancelled In STTask"));
        }
        else
        {
            DEBUG_LOG(TEXT("Ability completed normally In STTask"));
        }
    });

    //어빌리티 활성화
    const bool bSuccess = InstanceData.AbilitySystemComponent->TryActivateAbility(InstanceData.AbilityHandle, true);

    if (bSuccess)
    {
        DEBUG_LOG(TEXT("Successfully activated ability: %s"), *InstanceData.AbilityToActivate->GetName());
        return EStateTreeRunStatus::Running;
    }
    else
    {
        DEBUG_LOG(TEXT("Failed to activate ability: %s"), *InstanceData.AbilityToActivate->GetName());
        return EStateTreeRunStatus::Failed;
    }
}

EStateTreeRunStatus FActivateAbilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    //어빌리티가 끝났는지 확인
    if (InstanceData.bAbilityEnded)
    {
        if (InstanceData.bAbilityCancelled)
        {
            DEBUG_LOG(TEXT("Task failed - Ability was cancelled"));
            return EStateTreeRunStatus::Failed;
        }
        else
        {
            DEBUG_LOG(TEXT("Task succeeded - Ability completed"));
            return EStateTreeRunStatus::Succeeded;
        }
    }

    return EStateTreeRunStatus::Running;
}

void FActivateAbilityTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    if (Transition.ChangeType != EStateTreeStateChangeType::Changed)
    {
        return;
    }

    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    //델리게이트 언바인딩
    if (InstanceData.AbilitySystemComponent && InstanceData.AbilityEndedHandle.IsValid())
    {
        InstanceData.AbilitySystemComponent->OnAbilityEnded.Remove(InstanceData.AbilityEndedHandle);
        InstanceData.AbilityEndedHandle.Reset();
    }

    //어빌리티가 아직 실행 중이면 취소
    if (InstanceData.AbilitySystemComponent && InstanceData.AbilityHandle.IsValid())
    {
        InstanceData.AbilitySystemComponent->CancelAbilityHandle(InstanceData.AbilityHandle);
        DEBUG_LOG(TEXT("Cancelled ability on exit"));
    }
}

#if WITH_EDITOR
FText FActivateAbilityTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return FText::FromString(TEXT("<b>Activate GAS Ability</b>"));
}
#endif
