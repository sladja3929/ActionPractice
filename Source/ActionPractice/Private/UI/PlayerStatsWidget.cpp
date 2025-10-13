#include "UI/PlayerStatsWidget.h"

#include "Characters/BossCharacter.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "GAS/AttributeSet/ActionPracticeAttributeSet.h"
#include "GAS/AttributeSet/BossAttributeSet.h"
#include "Kismet/GameplayStatics.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogPlayerStatsWidget, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogPlayerStatsWidget, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

void UPlayerStatsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HealthBar)
	{
		HealthBar->SetPercent(1.0f);
	}

	if (HealthDamageBar)
	{
		HealthDamageBar->SetPercent(1.0f);
	}

	if (StaminaBar)
	{
		StaminaBar->SetPercent(1.0f);
	}

	if (StaminaDamageBar)
	{
		StaminaDamageBar->SetPercent(1.0f);
	}

	if (BossHealthBar)
	{
		BossHealthBar->SetPercent(1.0f);
	}

	if (BossHealthDamageBar)
	{
		BossHealthDamageBar->SetPercent(1.0f);
	}

	DEBUG_LOG(TEXT("PlayerStatsWidget Constructed"));
}

void UPlayerStatsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (AttributeSet)
	{
		UpdateHealth(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
		UpdateStamina(AttributeSet->GetStamina(), AttributeSet->GetMaxStamina());
	}

	if (BossAttributeSet)
	{
		UpdateBossHealth(BossAttributeSet->GetHealth(), BossAttributeSet->GetMaxHealth());
	}
	
	UpdateDamageBars(InDeltaTime);
}

void UPlayerStatsWidget::SetAttributeSet(UActionPracticeAttributeSet* InAttributeSet)
{
	if (!InAttributeSet)
	{
		DEBUG_LOG(TEXT("AttributeSet Invalid!"));
		return;
	}
	
	AttributeSet = InAttributeSet;
	SetBossAttributeSet();
	UnbindAttributeDelegates();
	BindAttributeDelegates();
	
	DEBUG_LOG(TEXT("AttributeSet set to PlayerStatsWidget"));
}

void UPlayerStatsWidget::UpdateHealth(float CurrentHealth, float MaxHealth)
{
	if (!HealthBar || MaxHealth <= 0.0f)
	{
		return;
	}
	
	float NewHealthPercent = CurrentHealth / MaxHealth;

	//HP 감소
	if (NewHealthPercent < CurrentHealthPercent)
	{
		HealthBar->SetPercent(NewHealthPercent);
		TargetHealthDamagePercent = NewHealthPercent;
		CurrentHealthDelayTimer = 0.0f;
	}
	
	//HP 증가
	else if (NewHealthPercent > CurrentHealthPercent)
	{
		HealthBar->SetPercent(NewHealthPercent);
		
		if (HealthDamageBar)
		{
			HealthDamageBar->SetPercent(NewHealthPercent);
		}

		TargetHealthDamagePercent = NewHealthPercent;
	}

	CurrentHealthPercent = NewHealthPercent;
}

void UPlayerStatsWidget::UpdateStamina(float CurrentStamina, float MaxStamina)
{
	if (!StaminaBar || MaxStamina <= 0.0f)
	{
		return;
	}
	
	float NewStaminaPercent = CurrentStamina / MaxStamina;

	//스테미나 감소
	if (NewStaminaPercent < CurrentStaminaPercent)
	{
		StaminaBar->SetPercent(NewStaminaPercent);

		//지속 감소일 경우 지연 X, 일반 바와 같이 감소
		if (NewStaminaPercent < CurrentStaminaPercent - 0.01f) TargetStaminaDamagePercent = NewStaminaPercent;
		else StaminaDamageBar->SetPercent(NewStaminaPercent);
		CurrentStaminaDelayTimer = 0.0f;
	}
	
	//스테미나 증가
	else if (NewStaminaPercent > CurrentStaminaPercent)
	{
		StaminaBar->SetPercent(NewStaminaPercent);
		
		if (StaminaDamageBar)
		{
			StaminaDamageBar->SetPercent(NewStaminaPercent);
		}

		TargetStaminaDamagePercent = NewStaminaPercent;
	}

	CurrentStaminaPercent = NewStaminaPercent;
}

void UPlayerStatsWidget::UpdateHealthBarSize(float MaxHealth)
{
    if (!HealthVerticalBox) return;
	
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(HealthVerticalBox->Slot);
	if (!CanvasSlot) return;
	
	FVector2D CurrentSize = CanvasSlot->GetSize();
    float BarWidth = MaxHealth * BarWidthPerHealth;

	CanvasSlot->SetSize(FVector2D(BarWidth, CurrentSize.Y));
    
    DEBUG_LOG(TEXT("Health Bar Size Updated: MaxHP=%.1f, BarWidth=%.1f"), ClampedMaxHealth, BarWidth);
}

void UPlayerStatsWidget::UpdateStaminaBarSize(float MaxStamina)
{
	if (!StaminaVerticalBox) return;
	
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(StaminaVerticalBox->Slot);
	if (!CanvasSlot) return;
	
	FVector2D CurrentSize = CanvasSlot->GetSize();
	float BarWidth = MaxStamina * BarWidthPerStamina;

	CanvasSlot->SetSize(FVector2D(BarWidth, CurrentSize.Y));
    
	DEBUG_LOG(TEXT("Stamina Bar Size Updated: MaxStamina=%.1f, BarWidth=%.1f"), ClampedMaxStamina, BarWidth);
}

void UPlayerStatsWidget::BindAttributeDelegates()
{
	if (!AttributeSet)
	{
		DEBUG_LOG(TEXT("No AttributeSet"));
		return;
	}
	
	UAbilitySystemComponent* ASC = AttributeSet->GetOwningAbilitySystemComponent();
    
	if (!ASC)
	{
		DEBUG_LOG(TEXT("Failed to get ASC from AttributeSet"));
		return;
	}
    
	MaxHealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute()).AddUObject(this, &UPlayerStatsWidget::OnMaxHealthChanged);
	MaxStaminaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxStaminaAttribute()).AddUObject(this, &UPlayerStatsWidget::OnMaxStaminaChanged);
    
	DEBUG_LOG(TEXT("Attribute Delegates Bound Successfully"));
    
	//초기 UI 업데이트
	UpdateHealthBarSize(AttributeSet->GetMaxHealth());
	UpdateStaminaBarSize(AttributeSet->GetMaxStamina());
}

void UPlayerStatsWidget::UnbindAttributeDelegates()
{
	if (!AttributeSet)
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = AttributeSet->GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}
    
	if (MaxHealthChangedHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
		MaxHealthChangedHandle.Reset();
	}
	
	if (MaxStaminaChangedHandle.IsValid())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxStaminaAttribute()).Remove(MaxStaminaChangedHandle);
		MaxStaminaChangedHandle.Reset();
	}
    
	DEBUG_LOG(TEXT("Attribute Delegates Unbound Successfully"));
}

void UPlayerStatsWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	DEBUG_LOG(TEXT("MaxHealth Changed: Old=%.2f, New=%.2f"), Data.OldValue, Data.NewValue);
    
	if (AttributeSet)
	{
		UpdateHealthBarSize(Data.NewValue);
	}
}

void UPlayerStatsWidget::OnMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	DEBUG_LOG(TEXT("MaxStamina Changed: Old=%.2f, New=%.2f"), Data.OldValue, Data.NewValue);
    
	if (AttributeSet)
	{
		UpdateStaminaBarSize(Data.NewValue);
	}
}

void UPlayerStatsWidget::UpdateDamageBars(float DeltaTime)
{
	if (HealthDamageBar && HealthDamageBar->GetPercent() > TargetHealthDamagePercent)
	{
		CurrentHealthDelayTimer += DeltaTime;

		if (CurrentHealthDelayTimer >= DamageBarDelayTime)
		{
			float CurrentPercent = HealthDamageBar->GetPercent();
			float NewPercent = FMath::FInterpTo(CurrentPercent, TargetHealthDamagePercent, DeltaTime, DamageBarLerpSpeed);
			HealthDamageBar->SetPercent(NewPercent);
			DEBUG_LOG(TEXT("HP Lerp Applied: NewPercent=%f"), NewPercent);
		}
	}

	if (StaminaDamageBar && StaminaDamageBar->GetPercent() > TargetStaminaDamagePercent)
	{
		CurrentStaminaDelayTimer += DeltaTime;

		if (CurrentStaminaDelayTimer >= DamageBarDelayTime)
		{
			float CurrentPercent = StaminaDamageBar->GetPercent();
			float NewPercent = FMath::FInterpTo(CurrentPercent, TargetStaminaDamagePercent, DeltaTime, DamageBarLerpSpeed);
			StaminaDamageBar->SetPercent(NewPercent);
			
			DEBUG_LOG(TEXT("ST Lerp Applied: NewPercent=%f"), NewPercent);
		}
	}

	if (BossHealthDamageBar && BossHealthDamageBar->GetPercent() > TargetBossHealthDamagePercent)
	{
		CurrentBossHealthDelayTimer += DeltaTime;

		if (CurrentBossHealthDelayTimer >= DamageBarDelayTime)
		{
			float CurrentPercent = BossHealthDamageBar->GetPercent();
			float NewPercent = FMath::FInterpTo(CurrentPercent, TargetBossHealthDamagePercent, DeltaTime, DamageBarLerpSpeed);
			BossHealthDamageBar->SetPercent(NewPercent);
			DEBUG_LOG(TEXT("Boss HP Lerp Applied: NewPercent=%f"), NewPercent);
		}
	}
}

void UPlayerStatsWidget::NativeDestruct()
{
	UnbindAttributeDelegates();
	
	Super::NativeDestruct();
}

void UPlayerStatsWidget::SetBossAttributeSet()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		DEBUG_LOG(TEXT("World is null"));
		return;
	}

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(World, ABossCharacter::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor->GetName().Contains(TEXT("BP_WoodGiant")))
		{
			ABossCharacter* BossCharacter = Cast<ABossCharacter>(Actor);
			if (BossCharacter)
			{
				BossAttributeSet = Cast<UBossAttributeSet>(BossCharacter->GetAttributeSet());
				if (BossAttributeSet)
				{
					DEBUG_LOG(TEXT("BossAttributeSet found and set from BP_WoodGiant"));
				}
				else
				{
					DEBUG_LOG(TEXT("Failed to cast to BossAttributeSet"));
				}
				return;
			}
		}
	}

	DEBUG_LOG(TEXT("BP_WoodGiant not found in world"));
}

void UPlayerStatsWidget::UpdateBossHealth(float CurrentHealth, float MaxHealth)
{
	if (!BossHealthBar || MaxHealth <= 0.0f)
	{
		return;
	}
	
	float NewHealthPercent = CurrentHealth / MaxHealth;

	//Boss HP 감소
	if (NewHealthPercent < CurrentBossHealthPercent)
	{
		BossHealthBar->SetPercent(NewHealthPercent);
		TargetBossHealthDamagePercent = NewHealthPercent;
		CurrentBossHealthDelayTimer = 0.0f;
	}
	
	//Boss HP 증가
	else if (NewHealthPercent > CurrentBossHealthPercent)
	{
		BossHealthBar->SetPercent(NewHealthPercent);
		
		if (BossHealthDamageBar)
		{
			BossHealthDamageBar->SetPercent(NewHealthPercent);
		}

		TargetBossHealthDamagePercent = NewHealthPercent;
	}

	CurrentBossHealthPercent = NewHealthPercent;
}
