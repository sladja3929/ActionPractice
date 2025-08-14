// Copyright Epic Games, Inc. All Rights Reserved.

#include "Public/Characters/ActionPracticeCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "GameplayAbilities/Public/Abilities/GameplayAbility.h"
#include "GAS/Abilities/AttackAbility.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AActionPracticeCharacter::AActionPracticeCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create Attribute Set
	AttributeSet = CreateDefaultSubobject<UActionPracticeAttributeSet>(TEXT("AttributeSet"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AActionPracticeCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize GAS
	InitializeAbilitySystem();

	EquipWeapon(LoadWeaponClassByName("BP_OneHandedSword"), false, false);
	EquipWeapon(LoadWeaponClassByName("BP_Shield"), true, false);
}

void AActionPracticeCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateLockOnCamera();
}

void AActionPracticeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// ===== Basic Input Actions (Direct Function Binding) =====
		// Movement
		if (IA_Move)
		{
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AActionPracticeCharacter::Move);
		}
        
		if (IA_Look)
		{
			EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AActionPracticeCharacter::Look);
		}

		if(IA_LockOn)
		{
			EnhancedInputComponent->BindAction(IA_LockOn, ETriggerEvent::Started, this, &AActionPracticeCharacter::ToggleLockOn);
		}

		if(IA_WeaponSwitch)
		{
			EnhancedInputComponent->BindAction(IA_WeaponSwitch, ETriggerEvent::Started, this, &AActionPracticeCharacter::WeaponSwitch);
		}

		// ===== GAS Ability Input Actions =====
		// Jump
		if (IA_Jump)
		{
			EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &AActionPracticeCharacter::OnJumpInput);
		}
        
		// Sprint (Hold)
		if (IA_Sprint)
		{
			EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AActionPracticeCharacter::OnSprintInput);
			EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AActionPracticeCharacter::OnSprintInputReleased);
		}
        
		// Crouch
		if (IA_Crouch)
		{
			EnhancedInputComponent->BindAction(IA_Crouch, ETriggerEvent::Started, this, &AActionPracticeCharacter::OnCrouchInput);
		}
        
		// Roll
		if (IA_Roll)
		{
			EnhancedInputComponent->BindAction(IA_Roll, ETriggerEvent::Started, this, &AActionPracticeCharacter::OnRollInput);
		}
        
		// Attack
		if (IA_Attack)
		{
			EnhancedInputComponent->BindAction(IA_Attack, ETriggerEvent::Started, this, &AActionPracticeCharacter::OnAttackInput);
		}
        
		// Block (Hold)
		if (IA_Block)
		{
			EnhancedInputComponent->BindAction(IA_Block, ETriggerEvent::Started, this, &AActionPracticeCharacter::OnBlockInput);
			EnhancedInputComponent->BindAction(IA_Block, ETriggerEvent::Completed, this, &AActionPracticeCharacter::OnBlockInputReleased);
		}
    }
}

#pragma region "Move Functions"
void AActionPracticeCharacter::Move(const FInputActionValue& Value)
{
	MovementInputVector = Value.Get<FVector2D>();

	//공격 어빌리티 중단
	if (MovementInputVector.Size() > 0.1f)
	{
		CancelActionForMove();
	}
	
	if (Controller != nullptr && !bIsRolling && !bIsAttacking)
	{
		if(!bIsSprinting && bIsLockOn && LockedOnTarget)
		{
			const FVector TargetLocation = LockedOnTarget->GetActorLocation();
			const FVector CharacterLocation = GetActorLocation();
            
			// 타겟을 향한 방향 계산
			FVector DirectionToTarget = TargetLocation - CharacterLocation;
			DirectionToTarget.Z = 0.0f; // 수평 방향만 고려
			DirectionToTarget.Normalize();
            
			// 타겟을 기준으로 한 이동 방향 계산
			const FRotator TargetRotation = DirectionToTarget.Rotation();
			const FVector RightDirection = FRotationMatrix(TargetRotation).GetUnitAxis(EAxis::Y);
			const FVector BackwardDirection = -DirectionToTarget; // 타겟 반대 방향
            
			// Strafe 이동 (좌우 이동)
			AddMovementInput(RightDirection, MovementInputVector.X);
            
			// 전후 이동 (타겟을 기준으로)
			AddMovementInput(BackwardDirection, -MovementInputVector.Y);
            
			// 캐릭터가 타겟을 바라보도록 회전
			SetActorRotation(TargetRotation);
		}

		else //일반적인 회전 이동
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
        
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        
			AddMovementInput(ForwardDirection, MovementInputVector.Y);
			AddMovementInput(RightDirection, MovementInputVector.X);
		}		
	}
}

void AActionPracticeCharacter::CancelActionForMove()
{
	if (!AbilitySystemComponent)
	{
		return;
	}
    
	// State.Recovering 태그가 없으면 어빌리티 캔슬 가능
	if (!AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.Recovering")))
	{
		// Ability.Attack 태그를 가진 어빌리티 취소
		FGameplayTagContainer CancelTags;
		CancelTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.Attack"));
		AbilitySystemComponent->CancelAbilities(&CancelTags);
	}
}

void AActionPracticeCharacter::StartSprint()
{
    // 달리기 가능 조건 체크
    if (bIsCrouching || bIsAttacking || bIsRolling || bIsBlocking)
    {
        return;
    }
    
    // 공중에서는 달리기 시작 불가
    if (GetCharacterMovement()->IsFalling())
    {
        return;
    }
    
    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * SprintSpeedMultiplier;
}

void AActionPracticeCharacter::StopSprint()
{
    bIsSprinting = false;
    
    // 앉아있는 상태면 앉기 속도로, 아니면 걷기 속도로
    if (bIsCrouching)
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * CrouchSpeedMultiplier;
    }
    else if (bIsBlocking)
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * BlockingSpeedMultiplier;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
}

void AActionPracticeCharacter::ToggleCrouch()
{
    // 액션 중에는 앉기 불가
    if (bIsAttacking || bIsRolling)
    {
        return;
    }
    
    if (bIsCrouching)
    {
        // 일어서기
        UnCrouch();
        bIsCrouching = false;
        
        // 달리고 있었다면 달리기 속도로, 아니면 걷기 속도로
        if (bIsSprinting)
        {
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * SprintSpeedMultiplier;
        }
        else if (bIsBlocking)
        {
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * BlockingSpeedMultiplier;
        }
        else
        {
            GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        }
    }
    else
    {
        // 앉기
        Crouch();
        bIsCrouching = true;
        
        // 달리기 중이었다면 달리기 해제
        if (bIsSprinting)
        {
            bIsSprinting = false;
        }
        
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * CrouchSpeedMultiplier;
    }
}

void AActionPracticeCharacter::StartJump()
{
    // 점프 가능 조건 체크
    if (bIsAttacking || bIsRolling || bIsBlocking)
    {
        return;
    }
    
    // 앉아있는 상태에서는 점프 대신 일어서기
    if (bIsCrouching)
    {
        ToggleCrouch();
        return;
    }
    
    // 기본 점프 함수 호출
    Jump();
}

void AActionPracticeCharacter::StopJump()
{
    // 기본 점프 중지 함수 호출
    StopJumping();
}

void AActionPracticeCharacter::Roll()
{
    if (!CanPerformAction() || !RollMontage)
    {
        return;
    }
    
    // 추가 조건 체크
    if (bIsAttacking || bIsBlocking || bIsCrouching)
    {
        return;
    }
    
    bIsRolling = true;
    
    // 달리기 중이었다면 해제
    if (bIsSprinting)
    {
        bIsSprinting = false;
    }
    
    // 입력 방향으로 캐릭터 회전
    if (MovementInputVector.Size() > 0.1f)
    {
        const FRotator ControlRotation = GetControlRotation();
        const FRotator YawRotation(0, ControlRotation.Yaw, 0);
        
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        
        const FVector RollDirection = 
            (ForwardDirection * MovementInputVector.Y + RightDirection * MovementInputVector.X).GetSafeNormal();
        
        if (RollDirection.Size() > 0.1f)
        {
            SetActorRotation(RollDirection.Rotation());
        }
    }
    
    // 몽타주 재생
    PlayAnimMontage(RollMontage);
    

}
#pragma endregion

#pragma region "Look Functions"
void AActionPracticeCharacter::Look(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;
	if(bIsLockOn && LockedOnTarget) return;
	
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void AActionPracticeCharacter::ToggleLockOn()
{
	if (bIsLockOn)
	{
		// 락온 해제
		bIsLockOn = false;
		LockedOnTarget = nullptr;
        
		// Spring Arm 설정 복원
		if (CameraBoom)
		{
			//CameraBoom->bUsePawnControlRotation = true;
		}
	}
	else
	{
		// 가장 가까운 적 찾기
		AActor* NearestTarget = FindNearestTarget();
		if (NearestTarget)
		{
			bIsLockOn = true;
			LockedOnTarget = NearestTarget;
            
			// 현재 Spring Arm 설정 저장
			if (CameraBoom)
			{                
				// 락온 시 카메라 설정 (필요시 조정 가능)
				// CameraBoom->TargetArmLength = 500.0f; // 원하는 거리로 조정
				// CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 100.0f); // 원하는 높이로 조정
			}
		}
	}
}

AActor* AActionPracticeCharacter::FindNearestTarget()
{
	TArray<AActor*> FoundTargets;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), FoundTargets);
    
	AActor* NearestTarget = nullptr;
	float NearestDistance = FLT_MAX;
    
	for (AActor* PotentialTarget : FoundTargets)
	{
		float Distance = FVector::Dist(GetActorLocation(), PotentialTarget->GetActorLocation());
		if (Distance < NearestDistance && Distance < 2000.0f) // 20미터 이내
		{
			NearestDistance = Distance;
			NearestTarget = PotentialTarget;
		}
	}
    
	return NearestTarget;
}

void AActionPracticeCharacter::UpdateLockOnCamera()
{
	if (bIsLockOn && LockedOnTarget && Controller && CameraBoom)
	{
		// 타겟과 캐릭터 위치
		const FVector TargetLocation = LockedOnTarget->GetActorLocation();
		const FVector CharacterLocation = GetActorLocation();
        
		// 캐릭터에서 타겟으로의 방향
		FVector DirectionToTarget = TargetLocation - CharacterLocation;
		DirectionToTarget.Normalize();
        
		// 카메라가 타겟과 캐릭터를 모두 볼 수 있는 각도 계산
		// 캐릭터 뒤에서 타겟을 바라보는 위치
		FVector CameraDirection = -DirectionToTarget;
        
		// 현재 Spring Arm의 길이를 사용하여 카메라 위치 계산
		FVector IdealCameraLocation = CharacterLocation + (CameraDirection * CameraBoom->TargetArmLength);
		IdealCameraLocation.Z += CameraBoom->SocketOffset.Z;
        
		// 카메라가 타겟을 바라보도록 회전 설정
		FRotator LookAtRotation = (TargetLocation - IdealCameraLocation).Rotation();
        
		// 컨트롤러 회전 설정
		Controller->SetControlRotation(LookAtRotation);
        
		// Spring Arm이 캐릭터 회전을 따르지 않도록 설정
		//CameraBoom->bInheritRoll = false;
	}
}
#pragma endregion

#pragma region "Block Functions"
// StartBlock 함수 구현
void AActionPracticeCharacter::StartBlock()
{
    if (!CanPerformAction() || bIsAttacking || bIsRolling)
    {
        return;
    }
    
    bIsBlocking = true;
    
    // 달리기 중이었다면 해제
    if (bIsSprinting)
    {
        bIsSprinting = false;
    }
    
    // 이동 속도 감소
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * BlockingSpeedMultiplier;
    
    // 시작 몽타주가 있다면 재생
    if (BlockStartMontage)
    {
        PlayAnimMontage(BlockStartMontage);
    }
}

// StopBlock 함수 구현  
void AActionPracticeCharacter::StopBlock()
{
    if (!bIsBlocking)
    {
        return;
    }
    
    bIsBlocking = false;
    
    // 속도 복구
    if (bIsCrouching)
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * CrouchSpeedMultiplier;
    }
    else if (bIsSprinting)
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * SprintSpeedMultiplier;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
    
    // 종료 몽타주가 있다면 재생
    if (BlockEndMontage)
    {
        PlayAnimMontage(BlockEndMontage);
    }
}
#pragma endregion

#pragma region "Weapon Functions"
void AActionPracticeCharacter::WeaponSwitch()
{
}

void AActionPracticeCharacter::EquipWeapon(TSubclassOf<AWeapon> NewWeaponClass, bool bIsLeftHand, bool bIsTwoHanded)
{
	if (!NewWeaponClass) return;

	if(bIsTwoHanded) UnequipWeapon(!bIsLeftHand);
	UnequipWeapon(bIsLeftHand);
    
	// 새 무기 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	
	AWeapon* NewWeapon = GetWorld()->SpawnActor<AWeapon>(NewWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    WeaponEnums type = NewWeapon->GetWeaponType();
	
	if (NewWeapon)
	{
		FString SocketString = bIsLeftHand ? "hand_l" : "hand_r";

		switch (type)
		{
		case WeaponEnums::StraightSword:
			SocketString += "_sword";
			break;

		case WeaponEnums::GreatSword:
			SocketString += "_greatsword";
			break;

		case WeaponEnums::Shield:
			SocketString += "_shield";
			break;
		}
		
		FName SocketName = FName(*SocketString);
		UE_LOG(LogTemp, Warning, TEXT("asdasdasda%s"), *SocketString);
		NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);

		if(bIsTwoHanded)
		{
			LeftWeapon = NewWeapon;
			RightWeapon = NewWeapon;
		}
		else if (bIsLeftHand)
		{
			LeftWeapon = NewWeapon;
		}
		
		else
		{
			RightWeapon = NewWeapon;
		}
	}
}

void AActionPracticeCharacter::UnequipWeapon(bool bIsLeftHand)
{
	AWeapon** WeaponToRemove = bIsLeftHand ? &LeftWeapon : &RightWeapon;
    
	if (*WeaponToRemove)
	{
		(*WeaponToRemove)->Destroy();
		*WeaponToRemove = nullptr;
	}
}

TSubclassOf<AWeapon> AActionPracticeCharacter::LoadWeaponClassByName(const FString& WeaponName)
{
	FString BlueprintPath = FString::Printf(TEXT("%s%s.%s_C"), 
										   *WeaponBlueprintBasePath, 
										   *WeaponName, 
										   *WeaponName);
	
	UClass* LoadedClass = LoadClass<AWeapon>(nullptr, *BlueprintPath);
	
	if (LoadedClass && LoadedClass->IsChildOf(AWeapon::StaticClass()))
	{
		return TSubclassOf<AWeapon>(LoadedClass);
	}

	UE_LOG(LogTemp, Warning, TEXT("Failed to load weapon class from path: %s"), *BlueprintPath);
	return nullptr;
}
#pragma endregion

#pragma region "GAS Input Functions"
void AActionPracticeCharacter::OnJumpInput()
{
	GASInputPressed(IA_Jump);
}

void AActionPracticeCharacter::OnSprintInput()
{
	GASInputPressed(IA_Sprint);
}

void AActionPracticeCharacter::OnSprintInputReleased()
{
	GASInputReleased(IA_Sprint);
}

void AActionPracticeCharacter::OnCrouchInput()
{
	GASInputPressed(IA_Crouch);
}

void AActionPracticeCharacter::OnRollInput()
{
	GASInputPressed(IA_Roll);
}

void AActionPracticeCharacter::OnAttackInput()
{
	GASInputPressed(IA_Attack);
}

void AActionPracticeCharacter::OnBlockInput()
{
	GASInputPressed(IA_Block);
}

void AActionPracticeCharacter::OnBlockInputReleased()
{
	GASInputReleased(IA_Block);
}
#pragma endregion

bool AActionPracticeCharacter::CanPerformAction() const
{
	return !bIsRolling && !GetCharacterMovement()->IsFalling();
}

#pragma region "GAS Functions"
UAbilitySystemComponent* AActionPracticeCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AActionPracticeCharacter::InitializeAbilitySystem()
{
	if (AbilitySystemComponent)
	{
		// Initialize the Ability System Component on the owning actor (this character)
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		// Set up the Attribute Set
		if (AttributeSet)
		{
			// Attributes are automatically initialized in the AttributeSet constructor
			// Additional setup can be done here if needed
		}
		
		// Grant startup abilities
		for (const auto& StartAbility : StartAbilities)
		{
			GiveAbility(StartAbility);
		}

		for (const auto& StartInputAbility : StartInputAbilities)
		{
			GiveAbility(StartInputAbility.Value);
		}

		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		
		for (const auto& StartEffect : StartEffects)
		{
			if (StartEffect)
			{
				FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(StartEffect, 1, EffectContext);
				if (SpecHandle.IsValid())
				{
					AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}
		}
	}
}

void AActionPracticeCharacter::GiveAbility(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (AbilitySystemComponent && AbilityClass)
	{
		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}

void AActionPracticeCharacter::GASInputPressed(const UInputAction* InputAction)
{
	if (!AbilitySystemComponent || !InputAction) return;
	
	TSubclassOf<UGameplayAbility>* AbilityClass = StartInputAbilities.Find(InputAction);
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(*AbilityClass);
	//FGameplayAbilitySpec* Spec = FindAbilitySpecFromClass(*AbilityClass);
	
	if (Spec)
	{
		Spec->InputPressed = true;
		
		if (Spec->IsActive())
		{
			AbilitySystemComponent->AbilitySpecInputPressed(*Spec);
		}

		else
		{
			AbilitySystemComponent->TryActivateAbility(Spec->Handle);
		}
	}
	
	else
	{
		// GAS 능력을 찾지 못했다면 기본 함수들 호출
		CallFallbackFunction(InputAction, true);
	}
}

void AActionPracticeCharacter::GASInputReleased(const UInputAction* InputAction)
{
	if (!AbilitySystemComponent || !InputAction) return;
	
	TSubclassOf<UGameplayAbility>* AbilityClass = StartInputAbilities.Find(InputAction);
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(*AbilityClass);
	//FGameplayAbilitySpec* Spec = FindAbilitySpecFromClass(*AbilityClass);

	if (Spec)
	{
		Spec->InputPressed = false;
		
		if (Spec->IsActive())
		{
			AbilitySystemComponent->AbilitySpecInputReleased(*Spec);
		}
	}
	
	else
	{
		// GAS 능력을 찾지 못했다면 기본 함수들 호출
		CallFallbackFunction(InputAction, false);
	}
}

void AActionPracticeCharacter::CallFallbackFunction(const UInputAction* InputAction, bool bIsPressed)
{
	if (InputAction == IA_Jump)
	{
		if (bIsPressed)
		{
			StartJump();
		}
		else
		{
			StopJump();
		}
	}
	else if (InputAction == IA_Sprint)
	{
		if (bIsPressed)
		{
			StartSprint();
		}
		else
		{
			StopSprint();
		}
	}
	else if (InputAction == IA_Crouch)
	{
		if (bIsPressed)
		{
			ToggleCrouch();
		}
		// Crouch는 토글이므로 Released 이벤트는 무시
	}
	else if (InputAction == IA_Roll)
	{
		if (bIsPressed)
		{
			Roll();
		}
		// Roll은 one-shot이므로 Released 이벤트는 무시
	}
	else if (InputAction == IA_Block)
	{
		if (bIsPressed)
		{
			StartBlock();
		}
		else
		{
			StopBlock();
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("No fallback function found for InputAction"));
	}
}
#pragma endregion