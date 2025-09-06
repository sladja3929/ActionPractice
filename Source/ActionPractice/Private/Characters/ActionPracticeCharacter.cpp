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
#include "Characters/InputBufferComponent.h"
#include "Items/Weapon.h"
#include "Items/WeaponCollisionComponent.h"

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
		bool bIsSprinting = AbilitySystemComponent->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateSprintingTag());
		
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
	bool bHasActiveAttackAbility = AbilitySystemComponent->HasMatchingGameplayTag(UGameplayTagsSubsystem::GetStateAttackingTag());
	
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

#pragma region "Weapon Functions"

TScriptInterface<IHitDetectionInterface> AActionPracticeCharacter::GetHitDetectionInterface() const
{
	return RightWeapon->GetCollisionComponent();
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
		UE_LOG(LogTemp, Warning, TEXT("Equiped Weapon: %s"), *SocketString);
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
	
	InputBufferComponent->bBufferActionReleased = false;
	
	if (Spec)
	{
		if (InputBufferComponent->bCanBufferInput) //다른 어빌리티가 수행중이고 입력 버퍼 가능할 때
		{
			InputBufferComponent->BufferNextAction(InputAction);
		}

		else
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
	}
}

void AActionPracticeCharacter::GASInputReleased(const UInputAction* InputAction)
{
	if (!AbilitySystemComponent || !InputAction) return;
	
	TSubclassOf<UGameplayAbility>* AbilityClass = StartInputAbilities.Find(InputAction);
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromClass(*AbilityClass);

	if (Spec)
	{	
		
		if (InputBufferComponent->bCanBufferInput) //다른 어빌리티가 수행중이고 입력 버퍼 가능할 때
		{
			InputBufferComponent->UnBufferHoldAction(InputAction);
		}
		
		else if (Spec->IsActive())
		{
			Spec->InputPressed = false;
			AbilitySystemComponent->AbilitySpecInputReleased(*Spec);
		}
	}
}
#pragma endregion