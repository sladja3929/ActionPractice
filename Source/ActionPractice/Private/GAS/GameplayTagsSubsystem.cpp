#include "GAS/GameplayTagsSubsystem.h"
#include "GAS/GameplayTagsDataAsset.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

void UGameplayTagsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// 데이터 에셋 로드 (에디터에서 설정한 데이터 에셋 경로)
	// TODO: 프로젝트 설정 또는 기본 경로에서 데이터 에셋 로드
	const FString DataAssetPath = TEXT("/Game/GAS/DA_GameplayTags");
	GameplayTagsDataAsset = LoadObject<UGameplayTagsDataAsset>(nullptr, *DataAssetPath);
	
	if (!GameplayTagsDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayTagsDataAsset could not be loaded from path: %s"), *DataAssetPath);
	}
}

UGameplayTagsSubsystem* UGameplayTagsSubsystem::Get()
{
	if (GEngine)
	{
		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			if (WorldContext.World() && WorldContext.WorldType != EWorldType::EditorPreview)
			{
				if (UGameInstance* GameInstance = WorldContext.World()->GetGameInstance())
				{
					return GameInstance->GetSubsystem<UGameplayTagsSubsystem>();
				}
			}
		}
	}
	
	return nullptr;
}

#pragma region "Static Accessor Functions"

// Ability Tags
const FGameplayTag& UGameplayTagsSubsystem::GetAbilityAttackTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetAbilityAttackTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilityAttackNormalTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetAbilityAttackNormalTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilityAttackChargeTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetAbilityAttackChargeTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilityRollTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetAbilityRollTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilitySprintTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetAbilitySprintTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilityJumpTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetAbilityJumpTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilityBlockTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetAbilityBlockTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

// State Tags
const FGameplayTag& UGameplayTagsSubsystem::GetStateAttackingTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetStateAttackingTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateBlockingTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetStateBlockingTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateRecoveringTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetStateRecoveringTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateStunnedTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetStateStunnedTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateInvincibleTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetStateInvincibleTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateJumpingTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetStateJumpingTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateSprintingTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetStateSprintingTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

// Event Tags
const FGameplayTag& UGameplayTagsSubsystem::GetEventNotifyEnableBufferInputTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetEventNotifyEnableBufferInputTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetEventNotifyActionRecoveryEndTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetEventNotifyActionRecoveryEndTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetEventNotifyResetComboTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetEventNotifyResetComboTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetEventNotifyChargeStartTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetEventNotifyChargeStartTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetEventActionPlayBufferTag()
{
	if (UGameplayTagsSubsystem* Subsystem = Get())
	{
		return Subsystem->GetEventActionPlayBufferTagInternal();
	}
	return FGameplayTag::EmptyTag;
}

#pragma endregion

#pragma region "Internal Tag Functions"

// Ability Tags
const FGameplayTag& UGameplayTagsSubsystem::GetAbilityAttackTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Ability_Attack;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilityAttackNormalTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Ability_Attack_Normal;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilityAttackChargeTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Ability_Attack_Charge;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilityRollTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Ability_Roll;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilitySprintTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Ability_Sprint;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilityJumpTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Ability_Jump;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetAbilityBlockTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Ability_Block;
	}
	return FGameplayTag::EmptyTag;
}

// State Tags
const FGameplayTag& UGameplayTagsSubsystem::GetStateAttackingTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->State_Attacking;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateBlockingTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->State_Blocking;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateRecoveringTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->State_Recovering;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateStunnedTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->State_Stunned;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateInvincibleTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->State_Invincible;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateJumpingTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->State_Jumping;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetStateSprintingTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->State_Sprinting;
	}
	return FGameplayTag::EmptyTag;
}

// Event Tags
const FGameplayTag& UGameplayTagsSubsystem::GetEventNotifyEnableBufferInputTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Event_Notify_EnableBufferInput;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetEventNotifyActionRecoveryEndTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Event_Notify_ActionRecoveryEnd;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetEventNotifyResetComboTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Event_Notify_ResetCombo;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetEventNotifyChargeStartTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Event_Notify_ChargeStart;
	}
	return FGameplayTag::EmptyTag;
}

const FGameplayTag& UGameplayTagsSubsystem::GetEventActionPlayBufferTagInternal() const
{
	if (GameplayTagsDataAsset)
	{
		return GameplayTagsDataAsset->Event_Action_PlayBuffer;
	}
	return FGameplayTag::EmptyTag;
}

#pragma endregion