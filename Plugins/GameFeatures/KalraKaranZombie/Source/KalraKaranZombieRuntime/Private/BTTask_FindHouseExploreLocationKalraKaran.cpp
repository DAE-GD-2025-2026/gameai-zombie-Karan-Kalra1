#include "BTTask_FindHouseExploreLocationKalraKaran.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

#include "../StudentPerceptorKalraKaran.h"

UBTTask_FindHouseExploreLocationKalraKaran::UBTTask_FindHouseExploreLocationKalraKaran()
{
	NodeName = TEXT("Find House Explore Location");
}

EBTNodeResult::Type UBTTask_FindHouseExploreLocationKalraKaran::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.f,
			FColor::Cyan,
			TEXT("Find House task is running")
		);
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	if (BB && BB->GetValueAsObject(TEXT("TargetItem")))
	{
		return EBTNodeResult::Failed;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return EBTNodeResult::Failed;

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
		return EBTNodeResult::Failed;

	UStudentPerceptorKalraKaran* Perceptor = GetPerceptor(AIController, Pawn);

	// First priority go to a remembered house.
	if (Perceptor && TryUseKnownHouse(OwnerComp, Pawn, Perceptor))
	{
		return EBTNodeResult::Succeeded;
	}

	// Fallback random exploration.
	if (TryUseRandomReachablePoint(OwnerComp, Pawn))
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

UStudentPerceptorKalraKaran* UBTTask_FindHouseExploreLocationKalraKaran::GetPerceptor(
	AAIController* AIController,
	APawn* Pawn) const
{
	if (AIController)
	{
		if (UStudentPerceptorKalraKaran* Perceptor =
			AIController->GetComponentByClass<UStudentPerceptorKalraKaran>())
		{
			return Perceptor;
		}
	}

	if (Pawn)
	{
		if (UStudentPerceptorKalraKaran* Perceptor =
			Pawn->GetComponentByClass<UStudentPerceptorKalraKaran>())
		{
			return Perceptor;
		}
	}

	return nullptr;
}

bool UBTTask_FindHouseExploreLocationKalraKaran::TryUseKnownHouse(
	UBehaviorTreeComponent& OwnerComp,
	APawn* Pawn,
	UStudentPerceptorKalraKaran* Perceptor) const
{
	if (!Pawn || !Perceptor)
		return false;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return false;

	const FVector PawnLocation = Pawn->GetActorLocation();

	// If we are already near a remembered house, mark it visited.
	Perceptor->MarkHouseVisitedNearLocation(PawnLocation, HouseReachRadius);

	// Only choose unvisited houses.
	AActor* House = Perceptor->GetBestUnvisitedHouse(PawnLocation);

	
	// Let random exploration run
	if (!House)
	{
		BB->ClearValue(TEXT("TargetHouse"));
		return false;
	}

	const float DistSq = FVector::DistSquared(
		PawnLocation,
		House->GetActorLocation());

	if (DistSq < FMath::Square(HouseReachRadius))
	{
		Perceptor->MarkHouseVisited(House);
		BB->ClearValue(TEXT("TargetHouse"));
		return false;
	}

	UNavigationSystemV1* NavSys =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(Pawn->GetWorld());

	if (!NavSys)
		return false;

	FNavLocation ProjectedLocation;

	if (NavSys->ProjectPointToNavigation(
		House->GetActorLocation(),
		ProjectedLocation,
		FVector(NavProjectionExtent, NavProjectionExtent, NavProjectionExtent)))
	{
		BB->SetValueAsVector(TargetLocationKey, ProjectedLocation.Location);

		//MarkHouseVisited task setter
		BB->SetValueAsObject(TEXT("TargetHouse"), House);

		return true;
	}

	return false;
}

bool UBTTask_FindHouseExploreLocationKalraKaran::TryUseRandomReachablePoint(
	UBehaviorTreeComponent& OwnerComp,
	APawn* Pawn) const
{
	if (!Pawn)
		return false;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return false;

	UNavigationSystemV1* NavSys =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(Pawn->GetWorld());

	if (!NavSys)
		return false;

	BB->ClearValue(TEXT("TargetHouse"));

	const FVector PawnLocation = Pawn->GetActorLocation();

	FNavLocation BestLocation;
	float BestScore = -FLT_MAX;
	bool bFound = false;

	for (int i = 0; i < 12; ++i)
	{
		FNavLocation Candidate;

		if (!NavSys->GetRandomReachablePointInRadius(
			PawnLocation,
			ExploreRadius,
			Candidate))
		{
			continue;
		}

		const float DistFromPawn =
			FVector::Dist(PawnLocation, Candidate.Location);

		// Avoid tiny local wandering.
		if (DistFromPawn < 1200.f)
			continue;

		// Prefer farther exploration points.
		float Score = DistFromPawn;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestLocation = Candidate;
			bFound = true;
		}
	}

	if (bFound)
	{
		BB->SetValueAsVector(TargetLocationKey, BestLocation.Location);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				1.f,
				FColor::Cyan,
				TEXT("Explore: far random point")
			);
		}

		return true;
	}

	// Last fallback any reachable point nearby.
	FNavLocation RandomLocation;
	if (NavSys->GetRandomReachablePointInRadius(
		PawnLocation,
		1000.f,
		RandomLocation))
	{
		BB->SetValueAsVector(TargetLocationKey, RandomLocation.Location);
		return true;
	}

	return false;
}