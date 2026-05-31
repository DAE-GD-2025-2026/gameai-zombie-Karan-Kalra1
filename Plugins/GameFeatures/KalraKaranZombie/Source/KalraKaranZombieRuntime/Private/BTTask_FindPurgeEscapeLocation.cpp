#include "BTTask_FindPurgeEscapeLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

UBTTask_FindPurgeEscapeLocation::UBTTask_FindPurgeEscapeLocation()
{
	NodeName = TEXT("Find Purge Escape Location");
}

EBTNodeResult::Type UBTTask_FindPurgeEscapeLocation::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
		return EBTNodeResult::Failed;

	APawn* Pawn = AIController->GetPawn();

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector PurgeLocation = BB->GetValueAsVector(PurgeZoneLocationKey);

	FVector AwayDirection = PawnLocation - PurgeLocation;
	AwayDirection.Z = 0.f;

	if (AwayDirection.IsNearlyZero())
	{
		AwayDirection = Pawn->GetActorForwardVector();
		AwayDirection.Z = 0.f;
	}

	AwayDirection.Normalize();

	UNavigationSystemV1* NavSys =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(Pawn->GetWorld());

	if (!NavSys)
		return EBTNodeResult::Failed;

	TArray<FVector> CandidateDirections;
	CandidateDirections.Add(AwayDirection);
	CandidateDirections.Add(AwayDirection.RotateAngleAxis(45.f, FVector::UpVector));
	CandidateDirections.Add(AwayDirection.RotateAngleAxis(-45.f, FVector::UpVector));
	CandidateDirections.Add(AwayDirection.RotateAngleAxis(90.f, FVector::UpVector));
	CandidateDirections.Add(AwayDirection.RotateAngleAxis(-90.f, FVector::UpVector));

	FVector BestLocation = FVector::ZeroVector;
	float BestScore = -FLT_MAX;
	bool bFound = false;

	for (const FVector& Direction : CandidateDirections)
	{
		const FVector DesiredLocation =
			PawnLocation + Direction * EscapeDistance;

		FNavLocation ProjectedLocation;

		if (!NavSys->ProjectPointToNavigation(
			DesiredLocation,
			ProjectedLocation,
			FVector(NavProjectionExtent, NavProjectionExtent, NavProjectionExtent)))
		{
			continue;
		}

		UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(
			Pawn->GetWorld(),
			PawnLocation,
			ProjectedLocation.Location,
			Pawn);

		if (!Path || !Path->IsValid() || Path->PathPoints.Num() <= 1)
			continue;

		const float DistanceFromPurge =
			FVector::Dist(ProjectedLocation.Location, PurgeLocation);

		const float DistanceFromPawn =
			FVector::Dist(ProjectedLocation.Location, PawnLocation);

		if (DistanceFromPawn < 300.f)
			continue;

		const float Score = DistanceFromPurge + DistanceFromPawn * 0.1f;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestLocation = ProjectedLocation.Location;
			bFound = true;
		}
	}

	if (bFound)
	{
		BB->SetValueAsVector(SafeLocationKey, BestLocation);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				1.f,
				FColor::Red,
				TEXT("Avoiding purge zone")
			);
		}

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}