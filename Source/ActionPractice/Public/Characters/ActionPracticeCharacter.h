// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "ActionPracticeCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UAbilitySystemComponent;
class UActionPracticeAttributeSet;
class UGameplayAbility;
class UInputBufferComponent;
class AWeapon;

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

	/** Input Buffer Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputBufferComponent* InputBufferComponent;

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
	FORCEINLINE const TMap<UInputAction*, TSubclassOf<UGameplayAbility>>& GetStartInputAbilities() const { return StartInputAbilities; }
	
	// ===== Input Buffer Interface =====
	FORCEINLINE UInputBufferComponent* GetInputBufferComponent() const { return InputBufferComponent; }
	
	// ===== Weapon Getter Functions =====
	FORCEINLINE AWeapon* GetLeftWeapon() const { return LeftWeapon; }
	FORCEINLINE AWeapon* GetRightWeapon() const { return RightWeapon; }

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* IA_ChargeAttack;
	
	// ===== State Variables =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action State")
	bool bIsLockOn = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action State")
	bool bIsSwitching = false;

	// ===== LockOn =====
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	AActor* LockedOnTarget = nullptr;
	
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
	
	// ===== Utility Functions =====
	
#pragma endregion

private:
#pragma region "Private Variables"
	
	// 회전 관련 변수
	FRotator TargetActionRotation;
	FRotator StartActionRotation;
	float CurrentRotationTime;
	float TotalRotationTime;
	bool bIsRotatingForAction;
	
#pragma endregion
	
#pragma region "Private Functions"

	void UpdateActionRotation(float DeltaTime);
	
#pragma endregion
};
