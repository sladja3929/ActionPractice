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

/**
 *	1. Base Component (camera)
 *	2. Input Action
 *	3. Animation
 */
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
	AActionPracticeCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== Return Function =====
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }	
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// ===== Weapon Function =====
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EquipWeapon(TSubclassOf<AWeapon> NewWeaponClass, bool bIsLeftHand = true);
    
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void UnequipWeapon(bool bIsLeftHand = true);
protected:
	// --------------------------------------------------
	
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom")
	float WalkSpeed = 400.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom")
	float SprintSpeedMultiplier = 1.5f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom")
	float CrouchSpeedMultiplier = 0.5f;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Custom")
	float BlockingSpeedMultiplier = 1.0f;

	// ===== State Variables =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsSprinting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsCrouching = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsBlocking = false;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsAttacking = false;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsRolling = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsLockOn = false;
    
	UPROPERTY(BlueprintReadOnly, Category = "State")
	int32 ComboCounter = 0;
    
	// ===== Combat Properties =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float ComboWindowTime = 0.7f;
	
	FTimerHandle ComboResetTimer;
	FVector2D MovementInputVector;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	AActor* LockedOnTarget = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<AWeapon> LeftWeaponClass;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<AWeapon> RightWeaponClass;
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	AWeapon* LeftWeapon = nullptr;
    
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	AWeapon* RightWeapon = nullptr;
	// -----------------------------------------------
	
	// ===== Locomotion Functions =====
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void ToggleCrouch();
	void StartJump();
	void StopJump();
	
	// ===== Combat Functions =====
	virtual void Attack();
	virtual void StartBlock();
	virtual void StopBlock();
	virtual void Roll();
	void ToggleLockOn();
	AActor* FindNearestTarget();
	void UpdateLockOnCamera();	
	
	// ===== Utility Functions =====
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool CanPerformAction() const;
    
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ResetCombo();
    
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
	
};

