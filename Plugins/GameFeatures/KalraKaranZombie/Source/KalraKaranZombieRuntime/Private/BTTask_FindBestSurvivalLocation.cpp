#include "BTTask_FindBestSurvivalLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

#include "../StudentPerceptor.h"

UBTTask_FindBestSurvivalLocation::UBTTask_FindBestSurvivalLocation()
{
	NodeName = TEXT("Find Best Survival Location");
}

EBTNodeResult::Type UBTTask_FindBestSurvivalLocation::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.f,
			FColor::Cyan,
			TEXT("Find Best Survival Location running")
		);
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return EBTNodeResult::Failed;

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
		return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	UStudentPerceptor* Perceptor = GetPerceptor(AIController, Pawn);

	const bool bNeedsHealing = BB->GetValueAsBool(NeedsHealingKey);
	const bool bHasWeapon = BB->GetValueAsBool(HasWeaponKey);

	const FVector PawnLocation = Pawn->GetActorLocation();

	// 1. If low health, run toward remembered medkit.
	if (Perceptor && bNeedsHealing)
	{
		if (AActor* Medkit = Perceptor->GetBestKnownMedkit(PawnLocation))
		{
			if (TryUseActorLocation(OwnerComp, Pawn, Medkit, SafeLocationKey, TargetItemKey))
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						1.f,
						FColor::Green,
						TEXT("Flee target: remembered medkit")
					);
				}

				return EBTNodeResult::Succeeded;
			}
		}
	}

	// 2. If unarmed, run toward remembered weapon.
	if (Perceptor && !bHasWeapon)
	{
		if (AActor* Weapon = Perceptor->GetBestKnownWeapon(PawnLocation))
		{
			if (TryUseActorLocation(OwnerComp, Pawn, Weapon, SafeLocationKey, TargetItemKey))
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						1.f,
						FColor::Green,
						TEXT("Flee target: remembered weapon")
					);
				}

				return EBTNodeResult::Succeeded;
			}
		}
	}

	const bool bShouldUseEmergencyShelter =
		(!bHasWeapon) || bNeedsHealing;

	// 3. If no useful item info exists, use a house as emergency shelter.
	if (Perceptor && bShouldUseEmergencyShelter)
	{
		if (AActor* House = Perceptor->GetBestUnvisitedHouse(PawnLocation))
		{
			AActor* Zombie = Cast<AActor>(BB->GetValueAsObject(TargetZombieKey));

			if (TryUseShelterLocation(OwnerComp, Pawn, House, Zombie, SafeLocationKey))
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						1.f,
						FColor::Green,
						TEXT("Flee target: emergency house shelter")
					);
				}

				return EBTNodeResult::Succeeded;
			}
		}
	}

	BB->ClearValue(TEXT("TargetHouse"));

	// rotate slightly so AIPerception can discover houses/items while escaping.
	FRotator ScanRotation = Pawn->GetActorRotation();
	ScanRotation.Yaw += FMath::RandBool() ? 90.f : -90.f;
	Pawn->SetActorRotation(ScanRotation);

	// 3. Otherwise run directly away from the zombie.
	AActor* Zombie = Cast<AActor>(BB->GetValueAsObject(TargetZombieKey));
	if (Zombie)
	{
		if (TryFindPointAwayFromZombie(OwnerComp, Pawn, Zombie))
		{
			return EBTNodeResult::Succeeded;
		}
	}

	// 4. Last fallback random reachable point.
	if (TryFindRandomFallback(OwnerComp, Pawn))
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

UStudentPerceptor* UBTTask_FindBestSurvivalLocation::GetPerceptor(
	AAIController* AIController,
	APawn* Pawn) const
{
	if (AIController)
	{
		if (UStudentPerceptor* Perceptor =
			AIController->GetComponentByClass<UStudentPerceptor>())
		{
			return Perceptor;
		}
	}

	if (Pawn)
	{
		if (UStudentPerceptor* Perceptor =
			Pawn->GetComponentByClass<UStudentPerceptor>())
		{
			return Perceptor;
		}
	}

	return nullptr;
}

bool UBTTask_FindBestSurvivalLocation::TryUseActorLocation(
	UBehaviorTreeComponent& OwnerComp,
	APawn* Pawn,
	AActor* Actor,
	FName LocationKey,
	FName OptionalTargetActorKey) const
{
	if (!Pawn || !Actor)
		return false;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return false;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return false;

	UNavigationSystemV1* NavSys =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(AIController->GetWorld());

	if (!NavSys)
		return false;

	const FVector PawnLocation = Pawn->GetActorLocation();

	FNavLocation ProjectedLocation;

	const bool bProjected = NavSys->ProjectPointToNavigation(
		Actor->GetActorLocation(),
		ProjectedLocation,
		FVector(NavProjectionExtent, NavProjectionExtent, NavProjectionExtent));

	if (!bProjected)
		return false;

	const float DistanceFromPawn = FVector::Dist(
		PawnLocation,
		ProjectedLocation.Location);

	// If the item/location is basically where we already are, don't use it as flee movement.
	if (DistanceFromPawn < MinMoveDistance)
		return false;

	UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(
		Pawn->GetWorld(),
		PawnLocation,
		ProjectedLocation.Location,
		Pawn);

	if (!Path || !Path->IsValid() || Path->PathPoints.Num() <= 1)
		return false;

	BB->SetValueAsVector(LocationKey, ProjectedLocation.Location);

	if (OptionalTargetActorKey != NAME_None)
	{
		BB->SetValueAsObject(OptionalTargetActorKey, Actor);
	}

	return true;
}

bool UBTTask_FindBestSurvivalLocation::TryUseShelterLocation(
	UBehaviorTreeComponent& OwnerComp,
	APawn* Pawn,
	AActor* ShelterActor,
	AActor* Zombie,
	FName LocationKey) const
{
	if (!Pawn || !ShelterActor)
		return false;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return false;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return false;

	UNavigationSystemV1* NavSys =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(Pawn->GetWorld());

	if (!NavSys)
		return false;

	const FVector PawnLocation = Pawn->GetActorLocation();
	const FVector ShelterLocation = ShelterActor->GetActorLocation();

	FNavLocation ProjectedLocation;

	const bool bProjected = NavSys->ProjectPointToNavigation(
		ShelterLocation,
		ProjectedLocation,
		FVector(NavProjectionExtent, NavProjectionExtent, NavProjectionExtent));

	if (!bProjected)
		return false;

	const float DistanceFromPawn = FVector::Dist(
		PawnLocation,
		ProjectedLocation.Location);

	if (DistanceFromPawn < MinMoveDistance)
		return false;

	UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(
		Pawn->GetWorld(),
		PawnLocation,
		ProjectedLocation.Location,
		Pawn);

	if (!Path || !Path->IsValid() || Path->PathPoints.Num() <= 1)
		return false;

	if (Zombie)
	{
		const float CurrentDistanceFromZombie =
			FVector::Dist(PawnLocation, Zombie->GetActorLocation());

		const float ShelterDistanceFromZombie =
			FVector::Dist(ProjectedLocation.Location, Zombie->GetActorLocation());

		// Do not flee to a shelter that is closer to the zombie than we are now.
		if (ShelterDistanceFromZombie <= CurrentDistanceFromZombie + 200.f)
			return false;
	}

	BB->SetValueAsVector(LocationKey, ProjectedLocation.Location);
	BB->SetValueAsObject(TEXT("TargetHouse"), ShelterActor);

	return true;
}

bool UBTTask_FindBestSurvivalLocation::TryFindPointAwayFromZombie(
	UBehaviorTreeComponent& OwnerComp,
	APawn* Pawn,
	AActor* Zombie) const
{
	if (!Pawn || !Zombie)
		return false;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return false;

	UNavigationSystemV1* NavSys =
		FNavigationSystem::GetCurrent<UNavigationSystemV1>(Pawn->GetWorld());

	if (!NavSys)
		return false;

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

	TArray<FVector> CandidateDirections;

	CandidateDirections.Add(AwayDirection);
	CandidateDirections.Add(AwayDirection.RotateAngleAxis(45.f, FVector::UpVector));
	CandidateDirections.Add(AwayDirection.RotateAngleAxis(-45.f, FVector::UpVector));
	CandidateDirections.Add(AwayDirection.RotateAngleAxis(90.f, FVector::UpVector));
	CandidateDirections.Add(AwayDirection.RotateAngleAxis(-90.f, FVector::UpVector));
	CandidateDirections.Add(AwayDirection.RotateAngleAxis(135.f, FVector::UpVector));
	CandidateDirections.Add(AwayDirection.RotateAngleAxis(-135.f, FVector::UpVector));

	FVector BestLocation = FVector::ZeroVector;
	float BestScore = -FLT_MAX;
	bool bFound = false;

	for (const FVector& Direction : CandidateDirections)
	{
		const FVector DesiredLocation = PawnLocation + Direction * FleeDistance;

		FNavLocation ProjectedLocation;

		const bool bProjected = NavSys->ProjectPointToNavigation(
			DesiredLocation,
			ProjectedLocation,
			FVector(NavProjectionExtent, NavProjectionExtent, NavProjectionExtent));

		if (!bProjected)
			continue;

		UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(
			Pawn->GetWorld(),
			PawnLocation,
			ProjectedLocation.Location,
			Pawn);

		if (!Path || !Path->IsValid() || Path->PathPoints.Num() <= 1)
			continue;

		const float DistanceFromPawn = FVector::Dist(
			PawnLocation,
			ProjectedLocation.Location);

		if (DistanceFromPawn < MinMoveDistance)
			continue;

		const float DistanceFromZombie = FVector::Dist(
			ProjectedLocation.Location,
			ZombieLocation);

		const float CurrentDistanceFromZombie = FVector::Dist(
			PawnLocation,
			ZombieLocation);

		// Do not pick a location that is closer to the zombie than we already are.
		if (DistanceFromZombie <= CurrentDistanceFromZombie)
			continue;

		// Prefer locations far from zombie and not too tiny a movement.
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

	if (bFound)
	{
		BB->SetValueAsVector(SafeLocationKey, BestLocation);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				1.f,
				FColor::Green,
				TEXT("Flee target: reachable point away from zombie")
			);
		}

		return true;
	}

	return false;
}

bool UBTTask_FindBestSurvivalLocation::TryFindRandomFallback(
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

	const FVector PawnLocation = Pawn->GetActorLocation();

	FNavLocation RandomLocation;

	if (NavSys->GetRandomReachablePointInRadius(
		PawnLocation,
		FleeDistance,
		RandomLocation))
	{
		const float DistanceFromPawn = FVector::Dist(
			PawnLocation,
			RandomLocation.Location);

		if (DistanceFromPawn >= MinMoveDistance)
		{
			BB->SetValueAsVector(SafeLocationKey, RandomLocation.Location);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					1.f,
					FColor::Yellow,
					TEXT("Flee target: random reachable fallback")
				);
			}

			return true;
		}
	}

	// Smaller fallback in case large radius fails inside buildings.
	if (NavSys->GetRandomReachablePointInRadius(
		PawnLocation,
		500.f,
		RandomLocation))
	{
		BB->SetValueAsVector(SafeLocationKey, RandomLocation.Location);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				1.f,
				FColor::Yellow,
				TEXT("Flee target: small random fallback")
			);
		}

		return true;
	}

	return false;
}