// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Public/Items/Weapon.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "ActionPracticeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All); 

UCLASS(abstract)
class AActionPracticeCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

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

	// ===== Weapon Functions =====
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	TSubclassOf<AWeapon> LoadWeaponClassByName(const FString& WeaponName);
	
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(TSubclassOf<AWeapon> NewWeaponClass, bool bIsLeftHand = true, bool bIsTwoHanded = false);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void UnequipWeapon(bool bIsLeftHand = true);

	// ===== Combo System Functions (Blueprint Callable for AnimNotify) =====	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CheckComboInput();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EnableComboInput();
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AttackRecoveryEnd();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ResetCombo();
	
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
	
#pragma endregion

#pragma region "Protected Functions"
	
	// ===== Locomotion Action Functions =====
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void ToggleCrouch();
	void StartJump();
	void StopJump();
	virtual void Roll();
	
	// ===== Combat Action Functions =====
	virtual void Attack();
	void SaveComboInput();
	void PlayAttackMontage();
	virtual void StartBlock();
	virtual void StopBlock();	
	void ToggleLockOn();
	AActor* FindNearestTarget();
	void UpdateLockOnCamera();
	void WeaponSwitch();
	
	// ===== Utility Functions =====
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanPerformAction() const;
    
	// ===== Blueprint Events =====
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnAttackStart(int32 ComboIndex);
    
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnAttackHit(AActor* HitActor, const FHitResult& HitResult);
    
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void OnRollStart();
    
	UFUNCTION(BlueprintImplementableEvent, Category = "Movement")
	void OnRollEnd();
    
	// ===== Montage Callbacks =====
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

#pragma endregion
};
