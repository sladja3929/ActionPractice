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

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AActionPracticeCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (GetMesh() && GetMesh()->GetSkeletalMeshAsset())
	{
		TArray<FName> SocketNames = GetMesh()->GetAllSocketNames();
		for (const FName& SocketName : SocketNames)
		{
			UE_LOG(LogTemp, Warning, TEXT("Available socket: %s"), *SocketName.ToString());
		}
	}
	// 몽타주 종료 델리게이트 바인딩
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &AActionPracticeCharacter::OnMontageEnded);
	}

	EquipWeapon(LeftWeaponClass, true);
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
		
		// Movement
		if (IA_Move)
		{
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AActionPracticeCharacter::Move);
		}
        
		if (IA_Look)
		{
			EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AActionPracticeCharacter::Look);
		}
        
		// Actions
		if (IA_Jump)
		{
			EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &AActionPracticeCharacter::StartJump);
			EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AActionPracticeCharacter::StopJump);
		}
        
		if (IA_Sprint)
		{
			EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AActionPracticeCharacter::StartSprint);
			EnhancedInputComponent->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AActionPracticeCharacter::StopSprint);
		}
        
		if (IA_Crouch)
		{
			EnhancedInputComponent->BindAction(IA_Crouch, ETriggerEvent::Started, this, &AActionPracticeCharacter::ToggleCrouch);
		}
        
		// Combat
		if (IA_Attack)
		{
			EnhancedInputComponent->BindAction(IA_Attack, ETriggerEvent::Started, this, &AActionPracticeCharacter::Attack);
		}
        
		if (IA_Block)
		{
			EnhancedInputComponent->BindAction(IA_Block, ETriggerEvent::Started, this, &AActionPracticeCharacter::StartBlock);
			EnhancedInputComponent->BindAction(IA_Block, ETriggerEvent::Completed, this, &AActionPracticeCharacter::StopBlock);
		}
        
		if (IA_Roll)
		{
			EnhancedInputComponent->BindAction(IA_Roll, ETriggerEvent::Started, this, &AActionPracticeCharacter::Roll);
		}

		if(IA_LockOn)
		{
			EnhancedInputComponent->BindAction(IA_LockOn, ETriggerEvent::Started, this, &AActionPracticeCharacter::ToggleLockOn);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AActionPracticeCharacter::Move(const FInputActionValue& Value)
{
	MovementInputVector = Value.Get<FVector2D>();
    
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

// 가장 가까운 타겟 찾기
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

void AActionPracticeCharacter::Attack()
{
	if (!CanPerformAction() || !AttackMontage) return;
    
	bIsAttacking = true;
    
	// 콤보 섹션 선택
	FName SectionName = FName(*FString::Printf(TEXT("Attack%d"), ComboCounter + 1));
    
	// 몽타주 재생
	PlayAnimMontage(AttackMontage, 1.0f, SectionName);
    
	// 블루프린트 이벤트
	OnAttackStart(ComboCounter);
    
	// 콤보 카운터 증가
	ComboCounter = (ComboCounter + 1) % 3;
    
	// 콤보 리셋 타이머
	GetWorldTimerManager().SetTimer(ComboResetTimer, this, &AActionPracticeCharacter::ResetCombo, ComboWindowTime, false);
}

bool AActionPracticeCharacter::CanPerformAction() const
{
	return !bIsRolling && !GetCharacterMovement()->IsFalling();
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

// Roll 함수 구현
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
    
    // 블루프린트 이벤트
    OnRollStart();
}

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

// ResetCombo 함수 구현
void AActionPracticeCharacter::ResetCombo()
{
    ComboCounter = 0;
}

// OnMontageEnded 함수 구현 (생성자에서 바인딩 필요)
void AActionPracticeCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == AttackMontage)
    {
        bIsAttacking = false;
        
        // 인터럽트된 경우 콤보 리셋
        if (bInterrupted)
        {
            ResetCombo();
        }
    }
    else if (Montage == RollMontage)
    {
        bIsRolling = false;
        OnRollEnd();
    }
}

void AActionPracticeCharacter::EquipWeapon(TSubclassOf<AWeapon> NewWeaponClass, bool bIsLeftHand)
{
	if (!NewWeaponClass) return;
    
	// 기존 무기 제거
	UnequipWeapon(bIsLeftHand);
    
	// 새 무기 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	
	AWeapon* NewWeapon = GetWorld()->SpawnActor<AWeapon>(NewWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    
	if (NewWeapon)
	{
		FName SocketName = TEXT("hand_r_sword");//bIsLeftHand ? TEXT("hand_l_shield") : TEXT("hand_r_sword");
		NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
        
		if (bIsLeftHand)
		{
			LeftWeapon = NewWeapon;
			LeftWeaponClass = NewWeaponClass;
		}
		else
		{
			RightWeapon = NewWeapon;
			RightWeaponClass = NewWeaponClass;
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