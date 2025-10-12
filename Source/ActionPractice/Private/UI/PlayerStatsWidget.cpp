#include "UI/PlayerStatsWidget.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "GAS/AttributeSet/ActionPracticeAttributeSet.h"

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

	UpdateDamageBars(InDeltaTime);
}

void UPlayerStatsWidget::SetAttributeSet(UActionPracticeAttributeSet* InAttributeSet)
{
	AttributeSet = InAttributeSet;
	DEBUG_LOG(TEXT("AttributeSet set to PlayerStatsWidget"));
}

void UPlayerStatsWidget::UpdateHealth(float CurrentHealth, float MaxHealth)
{
	if (!HealthBar || MaxHealth <= 0.0f)
	{
		return;
	}

	UpdateHealthBarSize(MaxHealth);
	
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

	UpdateStaminaBarSize(MaxStamina);
	
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
}