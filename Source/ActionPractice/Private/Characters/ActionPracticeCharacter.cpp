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
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "GAS/ActionPracticeAttributeSet.h"
#include "GameplayAbilities/Public/Abilities/GameplayAbility.h"
#include "GAS/GameplayTagsSubsystem.h"
#include "Input/InputBufferComponent.h"
#include "GAS/Abilities/ActionPracticeGameplayAbility.h"
#include "Input/InputActionDataAsset.h"
#include "Items/Weapon.h"
#include "Items/WeaponAttackTraceComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

// 디버그 로그 활성화/비활성화 (0: 비활성화, 1: 활성화)
#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
#define DEBUG_LOG(Format, ...) UE_LOG(LogAbilitySystemComponent, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

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

	// Create Input Buffer Component
	InputBufferComponent = CreateDefaultSubobject<UInputBufferComponent>(TEXT("InputBufferComponent"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AActionPracticeCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize GAS
	InitializeAbilitySystem();

	EquipWeapon(LoadWeaponClassByName("BP_StraightSword"), false, false);
	EquipWeapon(LoadWeaponClassByName("BP_Shield"), true, false);
}

void AActionPracticeCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateLockOnCamera();
	
	if (bIsRotatingForAction)
	{
		UpdateActionRotation(DeltaSeconds);
	}
}

void AActionPracticeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// ===== Basic Input Actions (Direct Function Binding) =====
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

		//Hold
		if (IA_ChargeAttack)
		{
			EnhancedInputComponent->BindAction(IA_ChargeAttack, ETriggerEvent::Started, this, &AActionPracticeCharacter::OnChargeAttackInput);
			EnhancedInputComponent->BindAction(IA_ChargeAttack, ETriggerEvent::Completed, this, &AActionPracticeCharacter::OnChargeAttackReleased);
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
	const FVector2D MovementVector = Value.Get<FVector2D>();

	//공격 어빌리티 중단
	if (MovementVector.Size() > 0.1f)
	{
		CancelActionForMove();
	}

	bool bIsRecovering = AbilitySystemComponent->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag());
	
	if (Controller != nullptr && !bIsRecovering)
	{
		bool bIsSprinting = AbilitySystemComponent->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateAbilitySprintingTag());
		
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
			AddMovementInput(RightDirection, MovementVector.X);
            
			// 전후 이동 (타겟을 기준으로)
			AddMovementInput(BackwardDirection, -MovementVector.Y);
            
			// 캐릭터가 타겟을 바라보도록 회전
			SetActorRotation(TargetRotation);
		}

		else //일반적인 회전 이동
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
        
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}		
	}
}

FVector2D AActionPracticeCharacter::GetCurrentMovementInput() const
{
	// PlayerController를 통해 어디서든 접근 가능
	APlayerController* PC = GetController<APlayerController>();
	if (PC && IA_Move)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// 특정 액션의 현재 값 조회
			FInputActionValue ActionValue = Subsystem->GetPlayerInput()->GetActionValue(IA_Move);
			return ActionValue.Get<FVector2D>();
		}
	}
	return FVector2D::ZeroVector;
}

void AActionPracticeCharacter::RotateCharacterToInputDirection(float RotateTime)
{
	// 현재 입력 값 가져오기
	FVector2D MovementInput = GetCurrentMovementInput();
    
	// 입력이 없으면 회전하지 않음
	if (MovementInput.IsZero())
	{
		return;
	}
    
	// 카메라의 Yaw 회전만 가져오기
	FRotator CameraRotation = FollowCamera->GetComponentRotation();
	FRotator CameraYaw = FRotator(0.0f, CameraRotation.Yaw, 0.0f);
    
	// 입력 벡터를 3D로 변환
	FVector InputDirection = FVector(MovementInput.Y, MovementInput.X, 0.0f);
    
	// 카메라 기준으로 입력 방향 변환
	FVector WorldDirection = CameraYaw.RotateVector(InputDirection);
	WorldDirection.Normalize();
    
	// 목표 회전 설정
	TargetActionRotation = FRotator(0.0f, WorldDirection.Rotation().Yaw, 0.0f);
    
	// 회전 시간이 0 이하면 즉시 회전
	if (RotateTime <= 0.0f)
	{
		SetActorRotation(TargetActionRotation);
		bIsRotatingForAction = false;
		return;
	}
    
	// 회전 각도 차이 계산
	float YawDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(
		GetActorRotation().Yaw, 
		TargetActionRotation.Yaw
	));
    
	// 회전 차이가 매우 작으면 즉시 완료
	if (YawDifference < 1.0f)
	{
		SetActorRotation(TargetActionRotation);
		bIsRotatingForAction = false;
		return;
	}
    
	// 스무스 회전 시작
	StartActionRotation = GetActorRotation();
	CurrentRotationTime = 0.0f;
	TotalRotationTime = RotateTime;
	bIsRotatingForAction = true;
}

void AActionPracticeCharacter::UpdateActionRotation(float DeltaTime)
{
	if (!bIsRotatingForAction)
	{
		return;
	}
    
	// 경과 시간 업데이트
	CurrentRotationTime += DeltaTime;
    
	// 회전 진행도 계산 (0~1)
	float Alpha = FMath::Clamp(CurrentRotationTime / TotalRotationTime, 0.0f, 1.0f);
    
	// 부드러운 커브 적용 (EaseInOut)
	Alpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);
    
	// 회전 보간
	FRotator NewRotation = FMath::Lerp(StartActionRotation, TargetActionRotation, Alpha);
	SetActorRotation(NewRotation);
    
	// 회전 완료 체크
	if (CurrentRotationTime >= TotalRotationTime)
	{
		// 정확한 목표 회전으로 설정
		SetActorRotation(TargetActionRotation);
		bIsRotatingForAction = false;
		CurrentRotationTime = 0.0f;
	}
}

void AActionPracticeCharacter::CancelActionForMove()
{
	if (!AbilitySystemComponent)
	{
		return;
	}
    
	// Attack 어빌리티가 활성화되어 있는지 확인
	bool bHasActiveAttackAbility = AbilitySystemComponent->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateAbilityAttackingTag());
	
	if (bHasActiveAttackAbility)
	{
		// State.Recovering 태그가 없으면 어빌리티 캔슬 가능 (ActionRecoveryEnd 이후)
		if (!AbilitySystemComponent->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateRecoveringTag()))
		{
			// Ability.Attack 태그를 가진 어빌리티 취소
			FGameplayTagContainer CancelTags;
			CancelTags.AddTag(UGameplayTagsSubsystem::GetAbilityAttackTag());
			AbilitySystemComponent->CancelAbilities(&CancelTags);
			DEBUG_LOG(TEXT("Attack Ability Cancelled by Move Input"));
		}
		else
		{
			DEBUG_LOG(TEXT("Attack Ability is in Recovering state - cannot cancel"));
		}
	}
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
		bIsLockOn = false;
		LockedOnTarget = nullptr;
        
		//Spring Arm 설정 복원
		if (CameraBoom)
		{
			CameraBoom->TargetArmLength = 400.0f;
			CameraBoom->SocketOffset = FVector::ZeroVector;
			CameraBoom->bUsePawnControlRotation = true;
		}
        
		DEBUG_LOG(TEXT("Lock-On Released"));
	}
	else
	{
		AActor* NearestTarget = FindNearestTarget();
		if (NearestTarget)
		{
			bIsLockOn = true;
			LockedOnTarget = NearestTarget;
			DEBUG_LOG(TEXT("Lock-On Target: %s"), *NearestTarget->GetName());
		}
		else
		{
			DEBUG_LOG(TEXT("No valid target found for Lock-On"));
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
		if (Distance < NearestDistance && Distance < 2000.0f)
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
        const FVector TargetLocation = LockedOnTarget->GetActorLocation();
        const FVector CharacterLocation = GetActorLocation();
        
        FVector DirectionToTarget = (TargetLocation - CharacterLocation).GetSafeNormal();
        
        //카메라 오프셋 설정
        const float CameraHorizontalOffset = 0.0f; //우측 오프셋
        const float CameraVerticalOffset = 100.0f; //추가 높이
        const float CameraBackOffset = 150.0f; //뒤로 물러나는 거리
        
        FVector RightVector = FVector::CrossProduct(FVector::UpVector, DirectionToTarget);
        RightVector.Normalize();
        
        //카메라 기본 위치 계산
        FVector MidPoint = CharacterLocation + (DirectionToTarget * FVector::Dist(CharacterLocation, TargetLocation) * 0.3f);
    	
        FVector CameraOffset = (-DirectionToTarget * CameraBackOffset) + 
                              (RightVector * CameraHorizontalOffset) + 
                              (FVector::UpVector * CameraVerticalOffset);
    	
        CameraBoom->TargetArmLength = 450.0f;
        CameraBoom->SocketOffset = FVector(0.0f, CameraHorizontalOffset, CameraVerticalOffset);
        

        FVector AdjustedTargetLocation = TargetLocation;
        AdjustedTargetLocation.Z += 50.0f;
  
        FVector LookAtPoint = (CharacterLocation + AdjustedTargetLocation) * 0.5f;
        LookAtPoint.Z = FMath::Max(CharacterLocation.Z, TargetLocation.Z) + 30.0f;
    	
        FRotator LookAtRotation = (LookAtPoint - CharacterLocation).Rotation();
    	
        LookAtRotation.Pitch = FMath::Clamp(LookAtRotation.Pitch, -25.0f, 15.0f);
        
        //부드러운 카메라
        FRotator CurrentRotation = Controller->GetControlRotation();
        FRotator SmoothedRotation = FMath::RInterpTo(CurrentRotation, LookAtRotation, 
                                                     GetWorld()->GetDeltaSeconds(), 5.0f);
    	
        Controller->SetControlRotation(SmoothedRotation);

        CameraBoom->bUsePawnControlRotation = true;
        CameraBoom->bInheritPitch = true;
        CameraBoom->bInheritYaw = true;
        CameraBoom->bInheritRoll = false;

        CameraBoom->bDoCollisionTest = true;
        CameraBoom->ProbeSize = 12.0f;
        CameraBoom->ProbeChannel = ECollisionChannel::ECC_Camera;
    }
}
#pragma endregion

#pragma region "Weapon Functions"

TScriptInterface<IHitDetectionInterface> AActionPracticeCharacter::GetHitDetectionInterface() const
{
	return RightWeapon->GetHitDetectionComponent();
}

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
    EWeaponEnums type = NewWeapon->GetWeaponType();
	
	if (NewWeapon && type != EWeaponEnums::None)
	{
		FString SocketString = bIsLeftHand ? "hand_l" : "hand_r";

		switch (type)
		{
		case EWeaponEnums::StraightSword:
			SocketString += "_sword";
			break;

		case EWeaponEnums::GreatSword:
			SocketString += "_greatsword";
			break;

		case EWeaponEnums::Shield:
			SocketString += "_shield";
			break;
		}
		
		FName SocketName = FName(*SocketString);
		DEBUG_LOG(TEXT("Equiped Weapon: %s"), *SocketString);
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

	DEBUG_LOG(TEXT("Failed to load weapon class from path: %s"), *BlueprintPath);
	return nullptr;
}
#pragma endregion

#pragma region "Attack Combo Functions"

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

void AActionPracticeCharacter::OnChargeAttackInput()
{
	GASInputPressed(IA_ChargeAttack);
}

void AActionPracticeCharacter::OnChargeAttackReleased()
{
	GASInputReleased(IA_ChargeAttack);
}

#pragma endregion

#pragma region "GAS Functions"
UAbilitySystemComponent* AActionPracticeCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AActionPracticeCharacter::InitializeAbilitySystem()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		if (AttributeSet)
		{
			// Attributes are automatically initialized in the AttributeSet constructor
			// Additional setup can be done here if needed
		}
		
		for (const auto& StartAbility : StartAbilities)
		{
			GiveAbility(StartAbility);
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

	TArray<FGameplayAbilitySpec*> TryActivateSpecs = FindAbilitySpecsWithInputAction(InputAction);
	if (TryActivateSpecs.IsEmpty()) return;
	
	InputBufferComponent->bBufferActionReleased = false;
	//다른 어빌리티가 수행중이고 입력 저장 가능할 때는 버퍼로 전달, Ability->InputPressed는 버퍼 이외의 구간에서만 사용
	if (InputBufferComponent->bCanBufferInput)
	{
		DEBUG_LOG(TEXT("Character: Buffer"));
		InputBufferComponent->BufferNextAction(InputAction);
	}

	else
	{
		for (auto& Spec : TryActivateSpecs)
		{
			if (Spec->IsActive())
			{
				Spec->InputPressed = true;
				AbilitySystemComponent->AbilitySpecInputPressed(*Spec);
			}

			else
			{
				Spec->InputPressed = true;
				AbilitySystemComponent->TryActivateAbility(Spec->Handle);
			}
		}
	}
}

void AActionPracticeCharacter::GASInputReleased(const UInputAction* InputAction)
{
	if (!AbilitySystemComponent || !InputAction) return;

	TArray<FGameplayAbilitySpec*> TryActivateSpecs = FindAbilitySpecsWithInputAction(InputAction);
	if (TryActivateSpecs.IsEmpty()) return;
	
	//다른 어빌리티가 수행중이고 입력 저장 가능할 때는 버퍼로 전달, Ability->InputPressed는 버퍼 이외의 구간에서만 사용
	if (InputBufferComponent->bCanBufferInput)
	{
		DEBUG_LOG(TEXT("Character: UnBuffer"));
		InputBufferComponent->UnBufferHoldAction(InputAction);
	}
	
	else
	{
		for (auto& Spec : TryActivateSpecs)
		{
			if (Spec->IsActive())
			{
				Spec->InputPressed = false;
				AbilitySystemComponent->AbilitySpecInputReleased(*Spec);
			}
		}
	}
}

TArray<FGameplayAbilitySpec*> AActionPracticeCharacter::FindAbilitySpecsWithInputAction(const UInputAction* InputAction)
{
	TArray<FGameplayAbilitySpec*> SameAssetSpecs;
	if (!AbilitySystemComponent) return SameAssetSpecs;
	
	const FInputActionAbilityRule* Rule = InputActionData->FindRuleByAction(InputAction);
	if (!Rule)
	{
		DEBUG_LOG(TEXT("FindAbilitySpecsWithInputAction: No Rule"));
		return SameAssetSpecs;
	}
	
	const FGameplayTagContainer* InputAssetTags = &Rule->AbilityAssetTags;
	if (!InputAssetTags)
	{
		DEBUG_LOG(TEXT("FindAbilitySpecsWithInputAction: No InputAssetTags"));
		return SameAssetSpecs;
	}
    
	for (auto& Spec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (Spec.Ability)
		{  
			if (Spec.Ability->GetAssetTags().HasAll(*InputAssetTags) || Spec.GetDynamicSpecSourceTags().HasAll(*InputAssetTags))
			{
				SameAssetSpecs.Add(&Spec);
			}
		}
	}

	return SameAssetSpecs;
}
#pragma endregion

