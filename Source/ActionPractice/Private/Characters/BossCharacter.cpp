#include "Characters/BossCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/AbilitySystemComponent/BossAbilitySystemComponent.h"
#include "GAS/AttributeSet/BossAttributeSet.h"
#include "AI/EnemyAIController.h"
#include "UI/BossHealthWidget.h"
#include "Characters/ActionPracticeCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Characters/HitDetection/EnemyAttackComponent.h"

#define ENABLE_DEBUG_LOG 0

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogBossCharacter, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogBossCharacter, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

ABossCharacter::ABossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	//Controller Settings
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//AI Controller Settings, AI Controller는 BP에서 할당
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	//Character Movement Settings
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	//GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CreateAbilitySystemComponent();
	CreateAttributeSet();

	//EnemyAttackComponent 생성
	EnemyAttackComponent = CreateDefaultSubobject<UEnemyAttackComponent>(TEXT("EnemyAttackComponent"));
}

TScriptInterface<IHitDetectionInterface> ABossCharacter::GetHitDetectionInterface() const
{
	return EnemyAttackComponent;
}

void ABossCharacter::CreateAbilitySystemComponent()
{
	AbilitySystemComponent = CreateDefaultSubobject<UBossAbilitySystemComponent>(TEXT("BossAbilitySystemComponent"));
}

void ABossCharacter::CreateAttributeSet()
{
	AttributeSet = CreateDefaultSubobject<UBossAttributeSet>(TEXT("BossAttributeSet"));
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();

	//AIController의 Perception 델리게이트 바인딩
	AEnemyAIController* BossController = GetEnemyAIController();
	if (BossController)
	{
		UAIPerceptionComponent* PerceptionComponent = BossController->GetPerceptionComponent();
		if (PerceptionComponent)
		{
			PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ABossCharacter::OnPlayerDetected);
			DEBUG_LOG(TEXT("Perception delegate bound to BossCharacter"));
		}
	}
	DEBUG_LOG(TEXT("BossChar Begin"));
}

void ABossCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveHealthWidget();

	Super::EndPlay(EndPlayReason);
}

void ABossCharacter::OnPlayerDetected(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

	//시각 감지 케이스
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		AActionPracticeCharacter* Player = Cast<AActionPracticeCharacter>(Actor);
		if (!Player) return;

		if (Stimulus.WasSuccessfullySensed())
		{
			DEBUG_LOG(TEXT("Player detected by Boss: %s"), *Actor->GetName());

			if (!bHealthWidgetActive)
			{
				DetectedPlayer = Player;
				CreateAndAttachHealthWidget();
			}
		}
		else
		{
			DEBUG_LOG(TEXT("Player lost by Boss: %s"), *Actor->GetName());

			if (Actor == DetectedPlayer.Get())
			{
				RemoveHealthWidget();
			}
		}
	}
}

void ABossCharacter::CreateAndAttachHealthWidget()
{
	if (bHealthWidgetActive)
	{
		DEBUG_LOG(TEXT("BossHealthWidget already active"));
		return;
	}

	if (!BossHealthWidgetClass)
	{
		DEBUG_LOG(TEXT("BossHealthWidgetClass is not set"));
		return;
	}
	
	BossHealthWidget = CreateWidget<UBossHealthWidget>(GetWorld(), BossHealthWidgetClass);
	if (!BossHealthWidget)
	{
		DEBUG_LOG(TEXT("Failed to create BossHealthWidget"));
		return;
	}

	UBossAttributeSet* BossAttributeSet = GetAttributeSet();
	if (BossAttributeSet)
	{
		BossHealthWidget->SetBossAttributeSet(BossAttributeSet);
	}
	
	BossHealthWidget->AddToViewport();

	bHealthWidgetActive = true;
	DEBUG_LOG(TEXT("BossHealthWidget created and attached"));
}

void ABossCharacter::RemoveHealthWidget()
{
	if (!bHealthWidgetActive)
	{
		return;
	}

	if (BossHealthWidget)
	{
		BossHealthWidget->RemoveFromParent();
		BossHealthWidget = nullptr;
		DEBUG_LOG(TEXT("BossHealthWidget removed"));
	}

	DetectedPlayer.Reset();
	bHealthWidgetActive = false;
}

void ABossCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateActionRotation(DeltaTime);
}

void ABossCharacter::RotateToTarget(const AActor* TargetActor, float RotateTime)
{
	if (!TargetActor)
	{
		DEBUG_LOG(TEXT("RotateToTarget: TargetActor is null"));
		return;
	}

	//타겟 방향 계산
	FVector Direction = TargetActor->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.0f; //수평 회전만 수행
	Direction.Normalize();

	//목표 회전 설정
	TargetActionRotation = FRotator(0.0f, Direction.Rotation().Yaw, 0.0f);

	//회전 시간이 0 이하면 즉시 회전
	if (RotateTime <= 0.0f)
	{
		SetActorRotation(TargetActionRotation);
		bIsRotatingForAction = false;
		DEBUG_LOG(TEXT("RotateToTarget: Instant rotation to %s"), *TargetActor->GetName());
		return;
	}

	//회전 각도 차이 계산
	float YawDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(
		GetActorRotation().Yaw,
		TargetActionRotation.Yaw
	));

	//회전 차이가 매우 작으면 즉시 완료
	if (YawDifference < 1.0f)
	{
		SetActorRotation(TargetActionRotation);
		bIsRotatingForAction = false;
		DEBUG_LOG(TEXT("RotateToTarget: Minor rotation to %s"), *TargetActor->GetName());
		return;
	}

	//스무스 회전 시작
	StartActionRotation = GetActorRotation();
	CurrentRotationTime = 0.0f;
	TotalRotationTime = RotateTime;
	bIsRotatingForAction = true;
	DEBUG_LOG(TEXT("RotateToTarget: Starting smooth rotation to %s over %.2f seconds"), *TargetActor->GetName(), RotateTime);
}

void ABossCharacter::UpdateActionRotation(float DeltaTime)
{
	if (!bIsRotatingForAction)
	{
		return;
	}
	
	CurrentRotationTime += DeltaTime;
	
	float Alpha = FMath::Clamp(CurrentRotationTime / TotalRotationTime, 0.0f, 1.0f);

	//부드러운 커브 적용
	Alpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

	//회전 보간
	FRotator NewRotation = FMath::Lerp(StartActionRotation, TargetActionRotation, Alpha);
	SetActorRotation(NewRotation);
	
	if (CurrentRotationTime >= TotalRotationTime)
	{
		//정확한 목표 회전으로 설정
		SetActorRotation(TargetActionRotation);
		bIsRotatingForAction = false;
		CurrentRotationTime = 0.0f;
		DEBUG_LOG(TEXT("UpdateActionRotation: Rotation completed"));
	}
}