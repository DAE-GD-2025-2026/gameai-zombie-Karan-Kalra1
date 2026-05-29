#include "BTTask_FindExploreLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_FindExploreLocation::UBTTask_FindExploreLocation()
{
	NodeName = TEXT("Find Explore Location");
}

EBTNodeResult::Type UBTTask_FindExploreLocation::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
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