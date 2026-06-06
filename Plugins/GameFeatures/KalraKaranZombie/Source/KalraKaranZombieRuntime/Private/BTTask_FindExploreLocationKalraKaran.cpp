#include "BTTask_FindExploreLocationKalraKaran.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_FindExploreLocationKalraKaran::UBTTask_FindExploreLocationKalraKaran()
{
	NodeName = TEXT("Find Explore Location");
}

EBTNodeResult::Type UBTTask_FindExploreLocationKalraKaran::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.f,
			FColor::Cyan,
			TEXT("Find Explore is running")
		);
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
		return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	UNavigationSystemV1* NavSys =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(AIController->GetWorld());

	if (!NavSys)
		return EBTNodeResult::Failed;

	const FVector Origin = AIController->GetPawn()->GetActorLocation();

	FNavLocation RandomLocation;
	if (NavSys->GetRandomReachablePointInRadius(Origin, ExploreRadius, RandomLocation))
	{
		BB->SetValueAsVector(TargetLocationKey, RandomLocation.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}