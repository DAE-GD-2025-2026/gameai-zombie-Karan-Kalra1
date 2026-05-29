#include "BTTask_FindSafeLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_FindSafeLocation::UBTTask_FindSafeLocation()
{
	NodeName = TEXT("Find Safe Location");
}

EBTNodeResult::Type UBTTask_FindSafeLocation::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
		return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	AActor* Zombie = Cast<AActor>(BB->GetValueAsObject(TargetZombieKey));
	if (!Zombie)
		return EBTNodeResult::Failed;

	APawn* Pawn = AIController->GetPawn();

	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector ZombieLocation = Zombie->GetActorLocation();

	FVector AwayDirection = PawnLocation - ZombieLocation;
	AwayDirection.Z = 0.f;

	if (AwayDirection.IsNearlyZero())
	{
		AwayDirection = Pawn->GetActorForwardVector();
		AwayDirection.Z = 0.f;
	}

	AwayDirection.Normalize();

	const FVector DesiredLocation = PawnLocation + AwayDirection * FleeDistance;

	UNavigationSystemV1* NavSys =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(AIController->GetWorld());

	if (!NavSys)
		return EBTNodeResult::Failed;

	FNavLocation ProjectedLocation;
	if (NavSys->ProjectPointToNavigation(DesiredLocation, ProjectedLocation))
	{
		BB->SetValueAsVector(SafeLocationKey, ProjectedLocation.Location);
		return EBTNodeResult::Succeeded;
	}

	FNavLocation RandomLocation;
	if (NavSys->GetRandomReachablePointInRadius(PawnLocation, FleeDistance, RandomLocation))
	{
		BB->SetValueAsVector(SafeLocationKey, RandomLocation.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}