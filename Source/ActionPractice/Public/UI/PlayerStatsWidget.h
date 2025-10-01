#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerStatsWidget.generated.h"

class UProgressBar;
class UActionPracticeAttributeSet;

/**
 * 엘든 링 스타일의 HP/스테미나 UI 위젯
 * HP와 스테미나가 감소하면 회색 바가 지연 후 점진적으로 줄어듦
 */
UCLASS()
class ACTIONPRACTICE_API UPlayerStatsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"
	//HP 바 (붉은색)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	//HP 지연 바 (회색)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthDamageBar;

	//스테미나 바 (초록색)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

	//스테미나 지연 바 (회색)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaDamageBar;
#pragma endregion

#pragma region "Public Functions"
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	//AttributeSet 설정
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetAttributeSet(UActionPracticeAttributeSet* InAttributeSet);

	//HP 업데이트
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealth(float CurrentHealth, float MaxHealth);

	//스테미나 업데이트
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateStamina(float CurrentStamina, float MaxStamina);
#pragma endregion

protected:
#pragma region "Protected Variables"
	//AttributeSet 참조
	UPROPERTY()
	TObjectPtr<UActionPracticeAttributeSet> AttributeSet;

	//현재 HP 퍼센트
	float CurrentHealthPercent;

	//현재 스테미나 퍼센트
	float CurrentStaminaPercent;

	//HP 지연 바의 목표 퍼센트
	float TargetHealthDamagePercent;

	//스테미나 지연 바의 목표 퍼센트
	float TargetStaminaDamagePercent;

	//HP 감소 후 대기 시간
	float HealthDamageDelay;

	//스테미나 감소 후 대기 시간
	float StaminaDamageDelay;

	//현재 HP 대기 타이머
	float CurrentHealthDelayTimer;

	//현재 스테미나 대기 타이머
	float CurrentStaminaDelayTimer;
#pragma endregion

#pragma region "Protected Functions"
	//지연 바를 부드럽게 감소
	void UpdateDamageBars(float DeltaTime);
#pragma endregion

private:
#pragma region "Private Variables"
	//Lerp 속도
	UPROPERTY(EditAnywhere, Category = "UI Settings", meta = (AllowPrivateAccess = "true"))
	float DamageBarLerpSpeed;

	//지연 시간 (초)
	UPROPERTY(EditAnywhere, Category = "UI Settings", meta = (AllowPrivateAccess = "true"))
	float DamageBarDelayTime;
#pragma endregion

#pragma region "Private Functions"
#pragma endregion
};