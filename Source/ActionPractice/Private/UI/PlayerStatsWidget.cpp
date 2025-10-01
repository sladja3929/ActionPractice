#include "UI/PlayerStatsWidget.h"
#include "Components/ProgressBar.h"
#include "GAS/ActionPracticeAttributeSet.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
	#define DEBUG_LOG(Format, ...) UE_LOG(LogTemp, Warning, Format, ##__VA_ARGS__)
#else
	#define DEBUG_LOG(Format, ...)
#endif

void UPlayerStatsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentHealthPercent = 1.0f;
	CurrentStaminaPercent = 1.0f;
	TargetHealthDamagePercent = 1.0f;
	TargetStaminaDamagePercent = 1.0f;
	CurrentHealthDelayTimer = 0.0f;
	CurrentStaminaDelayTimer = 0.0f;

	if (DamageBarLerpSpeed == 0.0f)
	{
		DamageBarLerpSpeed = 2.0f;
	}

	if (DamageBarDelayTime == 0.0f)
	{
		DamageBarDelayTime = 0.5f;
	}

	if (HealthBar)
	{
		HealthBar->SetPercent(1.0f);
		HealthBar->SetFillColorAndOpacity(FLinearColor::Red);
	}

	if (HealthDamageBar)
	{
		HealthDamageBar->SetPercent(1.0f);
		HealthDamageBar->SetFillColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
	}

	if (StaminaBar)
	{
		StaminaBar->SetPercent(1.0f);
		StaminaBar->SetFillColorAndOpacity(FLinearColor::Green);
	}

	if (StaminaDamageBar)
	{
		StaminaDamageBar->SetPercent(1.0f);
		StaminaDamageBar->SetFillColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
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

	float NewHealthPercent = CurrentHealth / MaxHealth;

	//HP가 감소한 경우
	if (NewHealthPercent < CurrentHealthPercent)
	{
		HealthBar->SetPercent(NewHealthPercent);
		TargetHealthDamagePercent = NewHealthPercent;
		CurrentHealthDelayTimer = 0.0f;
	}
	
	//HP가 회복된 경우
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

	//스테미나가 감소한 경우
	if (NewStaminaPercent < CurrentStaminaPercent)
	{
		StaminaBar->SetPercent(NewStaminaPercent);
		TargetStaminaDamagePercent = NewStaminaPercent;
		CurrentStaminaDelayTimer = 0.0f;
	}
	
	//스테미나가 회복된 경우
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

void UPlayerStatsWidget::UpdateDamageBars(float DeltaTime)
{
	//HP 지연 바 업데이트
	if (HealthDamageBar && HealthDamageBar->GetPercent() > TargetHealthDamagePercent)
	{
		CurrentHealthDelayTimer += DeltaTime;

		if (CurrentHealthDelayTimer >= DamageBarDelayTime)
		{
			float CurrentPercent = HealthDamageBar->GetPercent();
			float NewPercent = FMath::FInterpTo(CurrentPercent, TargetHealthDamagePercent, DeltaTime, DamageBarLerpSpeed);
			HealthDamageBar->SetPercent(NewPercent);
		}
	}

	//스테미나 지연 바 업데이트
	if (StaminaDamageBar && StaminaDamageBar->GetPercent() > TargetStaminaDamagePercent)
	{
		CurrentStaminaDelayTimer += DeltaTime;

		if (CurrentStaminaDelayTimer >= DamageBarDelayTime)
		{
			float CurrentPercent = StaminaDamageBar->GetPercent();
			float NewPercent = FMath::FInterpTo(CurrentPercent, TargetStaminaDamagePercent, DeltaTime, DamageBarLerpSpeed);
			StaminaDamageBar->SetPercent(NewPercent);
		}
	}
}