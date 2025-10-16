#include "AI/BossAIController.h"
#include "Components/StateTreeAIComponent.h"
#include "Characters/BossCharacter.h"

#define ENABLE_DEBUG_LOG 1

#if ENABLE_DEBUG_LOG
	DEFINE_LOG_CATEGORY_STATIC(LogBossAIController, Log, All);
#define DEBUG_LOG(Format, ...) UE_LOG(LogBossAIController, Warning, Format, ##__VA_ARGS__)
#else
#define DEBUG_LOG(Format, ...)
#endif

ABossAIController::ABossAIController()
{
	//StateTree Component 생성
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAIComponent"));
	check(StateTreeAIComponent);

	//AI 로직 자동 시작
	bStartAILogicOnPossess = true;
	
	//EnvQuery 등을 위한 Pawn 부착
	bAttachToPawn = true;
}

void ABossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	BossCharacter = Cast<ABossCharacter>(InPawn);
	if (!BossCharacter.IsValid())
	{
		DEBUG_LOG(TEXT("Failed to cast Pawn to BossCharacter"));
		return;
	}

	if (StateTreeAIComponent && StateTreeAsset)
	{
		//StateTreeComponent가 자동으로 Context를 관리
		//Schema에서 정의한 Context는 State Tree 에디터에서 바인딩
		StateTreeAIComponent->SetStateTree(StateTreeAsset);
		StateTreeAIComponent->StartLogic();
		DEBUG_LOG(TEXT("State Tree Started for %s"), *GetName());
	}
}

void ABossAIController::OnUnPossess()
{
	if (StateTreeAIComponent)
	{
		StateTreeAIComponent->StopLogic("Unpossessed");
	}

	BossCharacter.Reset();
	
	Super::OnUnPossess();
}