#include "UpdateSurvivorWorldState.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "../StudentPerceptor.h"

// Starter project includes
#include "Survivor/SurvivorPawn.h"
#include "Zombies/BaseZombie.h"
#include "Items/BaseItem.h"
#include "Common/HealthComponent.h"
#include "Common/StaminaComponent.h"
#include "Common/InventoryComponent.h"

UUpdateSurvivorWorldState::UUpdateSurvivorWorldState()
{
	NodeName = TEXT("Update Survivor World State");
	Interval = 0.25f;
	RandomDeviation = 0.05f;
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

	AActor* NearestZombie = nullptr;
	AActor* BestItem = nullptr;

	if (Perceptor)
	{
		const FVector PawnLocation = Pawn->GetActorLocation();

		NearestZombie = Perceptor->GetNearestKnownZombie(PawnLocation);
		BestItem = Perceptor->GetBestKnownItem(PawnLocation);
	}

	BB->SetValueAsObject(TEXT("TargetZombie"), NearestZombie);
	BB->SetValueAsObject(TEXT("TargetItem"), BestItem);

	const bool bHasWeapon = HasUsefulWeapon(Pawn);
	const bool bHasMedkit = HasItemType(Pawn, TEXT("Medkit"));
	const bool bHasFood = HasItemType(Pawn, TEXT("Food"));

	BB->SetValueAsBool(TEXT("HasWeapon"), bHasWeapon);
	BB->SetValueAsBool(TEXT("HasMedkit"), bHasMedkit);
	BB->SetValueAsBool(TEXT("HasFood"), bHasFood);

	float HealthValue = 1.0f;
	float StaminaValue = 1.0f;

	if (UHealthComponent* Health = Pawn->GetComponentByClass<UHealthComponent>())
	{
		HealthValue = Health->GetHealth();
	}

	if (UStaminaComponent* Stamina = Pawn->GetComponentByClass<UStaminaComponent>())
	{
		StaminaValue = Stamina->GetCurrentStamina();
	}

	BB->SetValueAsFloat(TEXT("HealthPercent"), HealthValue);
	BB->SetValueAsFloat(TEXT("StaminaPercent"), StaminaValue);

	

	const bool bNeedsHealing = HealthValue < 4.5f;
	const bool bNeedsStamina = StaminaValue < 3.5f;

	BB->SetValueAsBool(TEXT("NeedsHealing"), bNeedsHealing);
	BB->SetValueAsBool(TEXT("NeedsStamina"), bNeedsStamina);

	bool bIsInDanger = false;

	if (NearestZombie)
	{
		const float DistSq = FVector::DistSquared(
			Pawn->GetActorLocation(),
			NearestZombie->GetActorLocation()
		);

		const bool bZombieVeryClose = DistSq < FMath::Square(600.f);
		const bool bZombieCloseAndLowHealth =
			DistSq < FMath::Square(1000.f) && bNeedsHealing;


		bIsInDanger =
			bZombieCloseAndLowHealth ||
			(bZombieVeryClose && !bHasWeapon);
	}

	BB->SetValueAsBool(TEXT("IsInDanger"), bIsInDanger);
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

	for (AActor* Item : Inventory->GetInventory())
	{
		if (!Item)
			continue;

		const FString Name = Item->GetName();

		if (Name.Contains(TEXT("Pistol")) ||
			Name.Contains(TEXT("Shotgun")))
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

	for (AActor* Item : Inventory->GetInventory())
	{
		if (!Item)
			continue;

		if (Item->GetName().Contains(ItemTypeName))
		{
			return true;
		}
	}

	return false;
}