#include "UpdateSurvivorWorldState.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "../StudentPerceptor.h"

// Starter project includes
#include "Survivor/SurvivorPawn.h"
#include "Zombies/BaseZombie.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "Common/InventoryComponent.h"

UUpdateSurvivorWorldState::UUpdateSurvivorWorldState()
{
	NodeName = TEXT("Update Survivor World State");
	Interval = 0.25f;
	RandomDeviation = 0.05f;

	// Important because we store stuck-detection state per service instance.
	bCreateNodeInstance = true;
}

void UUpdateSurvivorWorldState::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return;

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
		return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return;

	UStudentPerceptor* Perceptor = GetPerceptor(AIController, Pawn);

	const float HealthValue = GetHealthValue(Pawn);
	const float StaminaValue = GetStaminaValue(Pawn);

	
	const bool bNeedsHealing = HealthValue < 4.5f;
	const bool bNeedsStamina = StaminaValue < 3.5f;

	const bool bHasWeapon = HasUsefulWeapon(Pawn);
	const bool bHasMedkit = HasItemType(Pawn, TEXT("Medkit"));
	const bool bHasFood = HasItemType(Pawn, TEXT("Food"));

	AActor* NearestZombie = nullptr;
	AActor* BestItem = nullptr;

	int32 NearbyZombieCount = 0;

	FVector KnownMedkitLocation = FVector::ZeroVector;
	FVector KnownWeaponLocation = FVector::ZeroVector;
	FVector KnownHouseLocation = FVector::ZeroVector;

	bool bHasKnownMedkitLocation = false;
	bool bHasKnownWeaponLocation = false;
	bool bHasKnownHouseLocation = false;

	if (Perceptor)
	{
		const FVector PawnLocation = Pawn->GetActorLocation();

		NearestZombie = Perceptor->GetNearestKnownZombie(PawnLocation, false);

		BestItem = Perceptor->GetBestKnownItem(
			PawnLocation,
			bNeedsHealing,
			bNeedsStamina,
			bHasWeapon
		);

		NearbyZombieCount = Perceptor->GetKnownZombieCount(1000.f, PawnLocation);

		bHasKnownMedkitLocation =
			Perceptor->GetBestKnownLocationOfType(
				EStudentMemoryType::Medkit,
				PawnLocation,
				KnownMedkitLocation);

		bHasKnownWeaponLocation =
			Perceptor->GetBestKnownLocationOfType(
				EStudentMemoryType::Weapon,
				PawnLocation,
				KnownWeaponLocation);

		bHasKnownHouseLocation =
			Perceptor->GetBestKnownLocationOfType(
				EStudentMemoryType::House,
				PawnLocation,
				KnownHouseLocation);
	}

	const float ThreatLevel = CalculateThreatLevel(
		Pawn,
		NearestZombie,
		NearbyZombieCount,
		bNeedsHealing,
		bHasWeapon
	);

	// ---------------------------------------------------------------------
	// Improved danger logic
	// ---------------------------------------------------------------------

	const bool bCriticalHealth = HealthValue < 25.f;
	const bool bLowHealth = HealthValue < 45.f;

	const bool bExtremeThreat = ThreatLevel >= 0.9f;
	const bool bDangerNoWeapon = !bHasWeapon && ThreatLevel >= 0.55f;
	const bool bDangerLowHealth = bLowHealth && ThreatLevel >= 0.45f;
	const bool bSurrounded = NearbyZombieCount >= 3;

	// Allow pickup to finish if standing beside an item.
	bool bNearTargetItem = false;
	if (AActor* TargetItem = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetItem"))))
	{
		const float DistSq = FVector::DistSquared(
			Pawn->GetActorLocation(),
			TargetItem->GetActorLocation());

		bNearTargetItem = DistSq < FMath::Square(175.f);
	}

	const bool bShouldFinishPickup =
		bNearTargetItem &&
		!bCriticalHealth &&
		!bExtremeThreat &&
		NearbyZombieCount < 3;

	const bool bIsInDanger =
		!bShouldFinishPickup &&
		(
			bExtremeThreat ||
			bDangerNoWeapon ||
			bDangerLowHealth ||
			bSurrounded
			);

	// ---------------------------------------------------------------------
	// Stuck detection
	// ---------------------------------------------------------------------

	const FVector CurrentLocation = Pawn->GetActorLocation();

	if (!bHasLastPawnLocation)
	{
		LastPawnLocation = CurrentLocation;
		TimeSinceMoved = 0.f;
		bHasLastPawnLocation = true;
	}
	else
	{
		const float MovedDistance = FVector::Dist(CurrentLocation, LastPawnLocation);

		if (MovedDistance < 20.f)
		{
			TimeSinceMoved += DeltaSeconds;
		}
		else
		{
			TimeSinceMoved = 0.f;
			LastPawnLocation = CurrentLocation;
		}
	}

	const bool bIsStuck = TimeSinceMoved > 3.f;

	// ---------------------------------------------------------------------
	// Blackboard updates
	// ---------------------------------------------------------------------

	BB->SetValueAsObject(TEXT("TargetZombie"), NearestZombie);
	BB->SetValueAsObject(TEXT("TargetItem"), BestItem);

	BB->SetValueAsBool(TEXT("HasWeapon"), bHasWeapon);
	BB->SetValueAsBool(TEXT("HasMedkit"), bHasMedkit);
	BB->SetValueAsBool(TEXT("HasFood"), bHasFood);

	BB->SetValueAsFloat(TEXT("HealthPercent"), HealthValue);
	BB->SetValueAsFloat(TEXT("StaminaPercent"), StaminaValue);

	BB->SetValueAsBool(TEXT("NeedsHealing"), bNeedsHealing);
	BB->SetValueAsBool(TEXT("NeedsStamina"), bNeedsStamina);

	BB->SetValueAsBool(TEXT("IsInDanger"), bIsInDanger);
	BB->SetValueAsBool(TEXT("IsStuck"), bIsStuck);

	BB->SetValueAsFloat(TEXT("ThreatLevel"), ThreatLevel);
	BB->SetValueAsInt(TEXT("NearbyZombieCount"), NearbyZombieCount);

	if (bHasKnownMedkitLocation)
	{
		BB->SetValueAsVector(TEXT("KnownMedkitLocation"), KnownMedkitLocation);
	}

	if (bHasKnownWeaponLocation)
	{
		BB->SetValueAsVector(TEXT("KnownWeaponLocation"), KnownWeaponLocation);
	}

	if (bHasKnownHouseLocation)
	{
		BB->SetValueAsVector(TEXT("KnownHouseLocation"), KnownHouseLocation);
	}
}

UStudentPerceptor* UUpdateSurvivorWorldState::GetPerceptor(
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

bool UUpdateSurvivorWorldState::HasUsefulWeapon(APawn* SurvivorPawn) const
{
	if (!SurvivorPawn)
		return false;

	UInventoryComponent* Inventory =
		SurvivorPawn->GetComponentByClass<UInventoryComponent>();

	if (!Inventory)
		return false;

	for (AActor* ItemActor : Inventory->GetInventory())
	{
		ABaseItem* Item = Cast<ABaseItem>(ItemActor);
		if (!Item)
			continue;

		if ((Item->GetItemType() == EItemType::Pistol ||
			Item->GetItemType() == EItemType::Shotgun) &&
			Item->GetValue() > 0)
		{
			return true;
		}
	}

	return false;
}

bool UUpdateSurvivorWorldState::HasItemType(
	APawn* SurvivorPawn,
	const FString& ItemTypeName) const
{
	if (!SurvivorPawn)
		return false;

	UInventoryComponent* Inventory =
		SurvivorPawn->GetComponentByClass<UInventoryComponent>();

	if (!Inventory)
		return false;

	for (AActor* ItemActor : Inventory->GetInventory())
	{
		ABaseItem* Item = Cast<ABaseItem>(ItemActor);
		if (!Item)
			continue;

		const EItemType Type = Item->GetItemType();

		if (ItemTypeName == TEXT("Medkit") && Type == EItemType::Medkit)
			return true;

		if (ItemTypeName == TEXT("Food") && Type == EItemType::Food)
			return true;

		if (ItemTypeName == TEXT("Pistol") && Type == EItemType::Pistol)
			return true;

		if (ItemTypeName == TEXT("Shotgun") && Type == EItemType::Shotgun)
			return true;
	}

	return false;
}

float UUpdateSurvivorWorldState::GetHealthValue(APawn* SurvivorPawn) const
{
	if (!SurvivorPawn)
		return 100.f;

	if (UHealthComponent* Health =
		SurvivorPawn->GetComponentByClass<UHealthComponent>())
	{
		return Health->GetHealth();
	}

	return 100.f;
}

float UUpdateSurvivorWorldState::GetStaminaValue(APawn* SurvivorPawn) const
{
	if (!SurvivorPawn)
		return 100.f;

	if (UStaminaComponent* Stamina =
		SurvivorPawn->GetComponentByClass<UStaminaComponent>())
	{
		return Stamina->GetCurrentStamina();
	}

	return 100.f;
}

float UUpdateSurvivorWorldState::CalculateThreatLevel(
	APawn* SurvivorPawn,
	AActor* NearestZombie,
	int32 NearbyZombieCount,
	bool bNeedsHealing,
	bool bHasWeapon) const
{
	if (!SurvivorPawn || !NearestZombie)
		return 0.f;

	const float Distance = FVector::Dist(
		SurvivorPawn->GetActorLocation(),
		NearestZombie->GetActorLocation()
	);

	float Threat = 0.f;

	// Distance threat: close zombies are dangerous.
	if (Distance < 300.f)
	{
		Threat += 0.8f;
	}
	else if (Distance < 600.f)
	{
		Threat += 0.5f;
	}
	else if (Distance < 1000.f)
	{
		Threat += 0.25f;
	}

	// Multiple zombies nearby increase danger.
	Threat += FMath::Clamp(
		static_cast<float>(NearbyZombieCount) * 0.15f,
		0.f,
		0.45f
	);

	// Low health makes the same situation more dangerous.
	if (bNeedsHealing)
	{
		Threat += 0.35f;
	}

	// Having a weapon makes danger more manageable.
	if (bHasWeapon)
	{
		Threat -= 0.25f;
	}

	return FMath::Clamp(Threat, 0.f, 1.f);
}