// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Public/Items/Weapon.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "ActionPracticeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UAbilitySystemComponent;
class UActionPracticeAttributeSet;
class UGameplayAbility;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All); 

UCLASS(abstract)
class AActionPracticeCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Ability System Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent;

	/** Attribute Set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	UActionPracticeAttributeSet* AttributeSet;

public:
#pragma region "Public Variables"
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FString WeaponBlueprintBasePath = TEXT("/Game/Items/BluePrint/");
	
#pragma endregion
	
#pragma region "Public Functions"
	
	AActionPracticeCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== Return Functions =====
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }	
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// ===== GAS Interface =====
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	FORCEINLINE UActionPracticeAttributeSet* GetAttributeSet() const { return AttributeSet; }

	// ===== Weapon Getter Functions =====
	FORCEINLINE AWeapon* GetLeftWeapon() const { return LeftWeapon; }
	FORCEINLINE AWeapon* GetRightWeapon() const { return RightWeapon; }

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
	
#pragma endregion

protected:
#pragma region "Protected Variables"
	
	// ====== Input Actions ======
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_Jump;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_Move;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_Look;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_LockOn;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_Sprint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_Crouch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_Roll;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_Attack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_Block;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_WeaponSwitch;
	
	// ===== Animation Montages =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
	class UAnimMontage* RollMontage;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
	class UAnimMontage* AttackMontage;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
	class UAnimMontage* BlockStartMontage;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montages")
	class UAnimMontage* BlockEndMontage;

	// ===== Movement Properties =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 400.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SprintSpeedMultiplier = 1.5f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float CrouchSpeedMultiplier = 0.5f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float BlockingSpeedMultiplier = 1.0f;

	// ===== State Variables =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action State")
	bool bIsSprinting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action State")
	bool bIsCrouching = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action State")
	bool bIsBlocking = false;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action State")
	bool bIsAttacking = false;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action State")
	bool bIsRolling = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action State")
	bool bIsLockOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action State")
	bool bIsSwitching = false;

	// ===== Combo System Variables =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bCanComboSave = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bComboInputSaved = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	int32 MaxComboCount = 3;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 ComboCounter = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	AActor* LockedOnTarget = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bCanABPInterruptMontage = false;
	
	FVector2D MovementInputVector;
	// ===== Weapon Properties =====
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AWeapon> WeaponClass = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	AWeapon* LeftWeapon = nullptr;
    
	UPROPERTY(BlueprintReadOnly, Category = "Weapon")
	AWeapon* RightWeapon = nullptr;

	// ===== GAS Properties =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> StartAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TMap<UInputAction*, TSubclassOf<UGameplayAbility>> StartInputAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> StartEffects;
	
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
	
	// ===== Legacy Action Functions =====
	void StartSprint();
	void StopSprint();
	void ToggleCrouch();
	void StartJump();
	void StopJump();
	virtual void Roll();	
	virtual void StartBlock();
	virtual void StopBlock();	
	
	// ===== GAS Input Handler Functions =====
	void OnJumpInput();
	void OnSprintInput();
	void OnSprintInputReleased();
	void OnCrouchInput();
	void OnRollInput();
	void OnAttackInput();
	void OnBlockInput();
	void OnBlockInputReleased();
	
	// ===== Utility Functions =====
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanPerformAction() const;

#pragma endregion

private:
#pragma region "Private Functions"
	
	void CallFallbackFunction(const UInputAction* InputAction, bool bIsPressed);
	
#pragma endregion
};
