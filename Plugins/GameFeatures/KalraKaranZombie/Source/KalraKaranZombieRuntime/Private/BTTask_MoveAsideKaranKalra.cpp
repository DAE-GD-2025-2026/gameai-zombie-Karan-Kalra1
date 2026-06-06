// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveAsideKaranKalra.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

UBTTask_MoveAsideKaranKalra::UBTTask_MoveAsideKaranKalra()
{
	NodeName = TEXT("Move Aside Karan Kalra");
}

EBTNodeResult::Type UBTTask_MoveAsideKaranKalra::ExecuteTask(
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

	AActor* Zombie = Cast<AActor>(BB->GetValueAsObject(TargetZombieKey));
	if (!Zombie)
		return EBTNodeResult::Failed;

	UNavigationSystemV1* NavSys =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(Pawn->GetWorld());

	if (!NavSys)
		return EBTNodeResult::Failed;

	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector ZombieLocation = Zombie->GetActorLocation();

	FVector Away = PawnLocation - ZombieLocation;
	Away.Z = 0.f;

	if (Away.IsNearlyZero())
	{
		Away = Pawn->GetActorForwardVector();
		Away.Z = 0.f;
	}

	Away.Normalize();

	const FVector Right =
		FVector::CrossProduct(Away, FVector::UpVector).GetSafeNormal();

	const FVector Left = -Right;

	TArray<FVector> Directions;
	Directions.Add(Right);
	Directions.Add(Left);
	Directions.Add((Away + Right).GetSafeNormal());
	Directions.Add((Away + Left).GetSafeNormal());
	Directions.Add(Away);

	FVector BestLocation = FVector::ZeroVector;
	float BestScore = -FLT_MAX;
	bool bFound = false;

	for (const FVector& Direction : Directions)
	{
		const FVector DesiredLocation =
			PawnLocation + Direction * StepDistance;

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

		const float DistanceFromZombie =
			FVector::Dist(ProjectedLocation.Location, ZombieLocation);

		const float DistanceFromPawn =
			FVector::Dist(ProjectedLocation.Location, PawnLocation);

		if (DistanceFromPawn < 250.f)
			continue;

		const float Score =
			DistanceFromZombie +
			DistanceFromPawn * 0.25f;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestLocation = ProjectedLocation.Location;
			bFound = true;
		}
	}

	if (!bFound)
	{
		return EBTNodeResult::Failed;
	}

	BB->SetValueAsVector(SafeLocationKey, BestLocation);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.f,
			FColor::Orange,
			TEXT("Move Aside: avoiding body block")
		);
	}

	return EBTNodeResult::Succeeded;
}