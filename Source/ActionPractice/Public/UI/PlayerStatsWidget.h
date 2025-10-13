#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerStatsWidget.generated.h"

class UBossAttributeSet;
struct FOnAttributeChangeData;
class UVerticalBox;
class UProgressBar;
class UActionPracticeAttributeSet;

UCLASS()
class ACTIONPRACTICE_API UPlayerStatsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox>HealthVerticalBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthDamageBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox>StaminaVerticalBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaDamageBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BossHealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BossHealthDamageBar;
	
#pragma endregion

#pragma region "Public Functions"
	
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetAttributeSet(UActionPracticeAttributeSet* InAttributeSet);
	
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealth(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateStamina(float CurrentStamina, float MaxStamina);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetBossAttributeSet();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateBossHealth(float CurrentHealth, float MaxHealth);
	
#pragma endregion

protected:
#pragma region "Protected Variables"
	
	UPROPERTY()
	TObjectPtr<UActionPracticeAttributeSet> AttributeSet;

	float CurrentHealthPercent = 1.0f;
	float CurrentStaminaPercent = 1.0f;
	
	float TargetHealthDamagePercent = 1.0f;
	float TargetStaminaDamagePercent = 1.0f;
	
	float HealthDamageDelay = 0.0f;
	float StaminaDamageDelay = 0.0f;
	
	float CurrentHealthDelayTimer = 0.0f;
	float CurrentStaminaDelayTimer = 0.0f;

	//Attribute Change Delegate
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle MaxStaminaChangedHandle;

	UPROPERTY()
	TObjectPtr<UBossAttributeSet> BossAttributeSet;
	float CurrentBossHealthPercent = 1.0f;
	float TargetBossHealthDamagePercent = 1.0f;
	float CurrentBossHealthDelayTimer = 0.0f;
#pragma endregion

#pragma region "Protected Functions"
	
	//지연 바 감소
	void UpdateDamageBars(float DeltaTime);

	void UpdateHealthBarSize(float MaxHealth);
	void UpdateStaminaBarSize(float MaxStamina);

	//델리게이트 바인드
	void BindAttributeDelegates();
	void UnbindAttributeDelegates();

	//콜백 함수
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxStaminaChanged(const FOnAttributeChangeData& Data);
#pragma endregion

private:
#pragma region "Private Variables"
	
	//지연 바 줄어드는 속도
	UPROPERTY(EditAnywhere, Category = "UI Settings", meta = (AllowPrivateAccess = "true"))
	float DamageBarLerpSpeed = 2.0f;

	//지연 바가 몇초 뒤에 줄어드는지
	UPROPERTY(EditAnywhere, Category = "UI Settings", meta = (AllowPrivateAccess = "true"))
	float DamageBarDelayTime = 0.5f;

	//체력 1당 바 길이
	UPROPERTY(EditAnywhere, Category = "UI Settings", meta = (AllowPrivateAccess = "true"))
	float BarWidthPerHealth = 0.5f;
	
	//스테미나 1당 바 길이
	UPROPERTY(EditAnywhere, Category = "UI Settings", meta = (AllowPrivateAccess = "true"))
	float BarWidthPerStamina = 3.0f;
	
#pragma endregion

#pragma region "Private Functions"
	
#pragma endregion
};