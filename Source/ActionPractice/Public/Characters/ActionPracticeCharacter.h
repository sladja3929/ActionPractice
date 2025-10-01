// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "ActionPracticeCharacter.generated.h"

class UInputActionDataAsset;
class IHitDetectionInterface;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UAbilitySystemComponent;
class UActionPracticeAttributeSet;
class UGameplayAbility;
class UInputBufferComponent;
class AWeapon;
class UPlayerStatsWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All); 

UCLASS(abstract)
class AActionPracticeCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
#pragma region "Public Variables"
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FString WeaponBlueprintBasePath = TEXT("/Game/Items/BluePrint/");

	// ===== Movement Properties =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 400.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintSpeedMultiplier = 1.5f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float CrouchSpeedMultiplier = 0.5f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float BlockingSpeedMultiplier = 1.0f;
#pragma endregion
	
#pragma region "Public Functions"
	
	AActionPracticeCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== Camera =====
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }	
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// ===== GAS Interface =====
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE UActionPracticeAttributeSet* GetAttributeSet() const { return AttributeSet; }
	FORCEINLINE const UInputActionDataAsset* GetInputActionData() const { return InputActionData; }
	
	// ===== Input Buffer Interface =====
	FORCEINLINE UInputBufferComponent* GetInputBufferComponent() const { return InputBufferComponent; }
	
	// ===== Weapon Getter Functions =====
	FORCEINLINE AWeapon* GetLeftWeapon() const { return LeftWeapon; }
	FORCEINLINE AWeapon* GetRightWeapon() const { return RightWeapon; }
	virtual TScriptInterface<IHitDetectionInterface> GetHitDetectionInterface() const;
	
	// ===== Input Helper Functions =====
	UFUNCTION(BlueprintPure, Category = "Input")
	FVector2D GetCurrentMovementInput() const;

	// ===== Character Rotation Functions =====
	UFUNCTION(BlueprintCallable, Category = "Character")
	void RotateCharacterToInputDirection(float RotationTime);


	// ===== Weapon Functions =====
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	TSubclassOf<AWeapon> LoadWeaponClassByName(const FString& WeaponName);
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(TSubclassOf<AWeapon> NewWeaponClass, bool bIsLeftHand = true, bool bIsTwoHanded = false);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void UnequipWeapon(bool bIsLeftHand = true);
	
	// ===== GAS Functions =====
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void InitializeAbilitySystem();

	UFUNCTION(BlueprintCallable, Category = "GAS")
	void GiveAbility(TSubclassOf<UGameplayAbility> AbilityClass);

	UFUNCTION(BlueprintCallable, Category = "GAS")
	void GASInputPressed(const UInputAction* InputAction);
	
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void GASInputReleased(const UInputAction* InputAction);

	TArray<FGameplayAbilitySpec*> FindAbilitySpecsWithInputAction(const UInputAction* InputAction);
#pragma endregion

protected:
#pragma region "Protected Variables"
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom = nullptr;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera = nullptr;

	/** Ability System Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	/** Attribute Set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UActionPracticeAttributeSet> AttributeSet = nullptr;

	/** Input Buffer Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputBufferComponent> InputBufferComponent = nullptr;

	// ===== Stats UI Properties =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPlayerStatsWidget> PlayerStatsWidgetClass;

	UPROPERTY()
	TObjectPtr<UPlayerStatsWidget> PlayerStatsWidget;

	// ===== Weapon Properties =====
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeapon> WeaponClass = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<AWeapon> LeftWeapon = nullptr;
    
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<AWeapon> RightWeapon = nullptr;

	// ===== GAS Properties =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> StartAbilities;
		
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> StartEffects;
	
	// ====== Input Actions ======
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Jump = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Move = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Look = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_LockOn = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Sprint = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Crouch = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Roll = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Attack = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_Block = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_WeaponSwitch = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_ChargeAttack = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputActionDataAsset> InputActionData = nullptr;
	
	// ===== State Variables =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action State")
	bool bIsLockOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action State")
	bool bIsSwitching = false;

	// ===== LockOn =====
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> LockedOnTarget = nullptr;
	
#pragma endregion

#pragma region "Protected Functions"
	
	// ===== Input Handler Functions =====
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ToggleLockOn();
	void WeaponSwitch();

	// ===== Input Handler Additional Functions =====
	void CancelActionForMove();
	AActor* FindNearestTarget();
	void UpdateLockOnCamera();
	
	// ===== GAS Input Handler Functions =====
	void OnJumpInput();
	void OnSprintInput();
	void OnSprintInputReleased();
	void OnCrouchInput();
	void OnRollInput();
	void OnAttackInput();
	void OnBlockInput();
	void OnBlockInputReleased();
	void OnChargeAttackInput();
	void OnChargeAttackReleased();
	
#pragma endregion

private:
#pragma region "Private Variables"
	
	//회전 관련 변수
	FRotator TargetActionRotation;
	FRotator StartActionRotation;
	float CurrentRotationTime = 0;
	float TotalRotationTime = 0;
	bool bIsRotatingForAction = false;
	
#pragma endregion
	
#pragma region "Private Functions"

	void UpdateActionRotation(float DeltaTime);
	
#pragma endregion
};
