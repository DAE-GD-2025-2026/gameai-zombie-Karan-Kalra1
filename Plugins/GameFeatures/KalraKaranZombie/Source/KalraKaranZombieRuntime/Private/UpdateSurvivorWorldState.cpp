#include "UpdateSurvivorWorldState.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

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

	AActor* NearestZombie = FindNearestZombie(Pawn);
	AActor* BestItem = FindBestItem(Pawn);

	BB->SetValueAsObject(TEXT("TargetZombie"), NearestZombie);
	BB->SetValueAsObject(TEXT("TargetItem"), BestItem);

	const bool bHasWeapon = HasUsefulWeapon(Pawn);
	const bool bHasMedkit = HasItemType(Pawn, TEXT("Medkit"));
	const bool bHasFood = HasItemType(Pawn, TEXT("Food"));

	BB->SetValueAsBool(TEXT("HasWeapon"), bHasWeapon);
	BB->SetValueAsBool(TEXT("HasMedkit"), bHasMedkit);
	BB->SetValueAsBool(TEXT("HasFood"), bHasFood);

	float HealthPercent = 1.0f;
	float StaminaPercent = 1.0f;

	if (UHealthComponent* Health = Pawn->GetComponentByClass<UHealthComponent>())
	{
		HealthPercent = Health->GetHealth();
	}

	if (UStaminaComponent* Stamina = Pawn->GetComponentByClass<UStaminaComponent>())
	{
		StaminaPercent = Stamina->GetCurrentStamina();
	}

	BB->SetValueAsFloat(TEXT("HealthPercent"), HealthPercent);
	BB->SetValueAsFloat(TEXT("StaminaPercent"), StaminaPercent);

	const bool bNeedsHealing = HealthPercent < 0.45f;
	const bool bNeedsStamina = StaminaPercent < 0.35f;

	BB->SetValueAsBool(TEXT("NeedsHealing"), bNeedsHealing);
	BB->SetValueAsBool(TEXT("NeedsStamina"), bNeedsStamina);

	bool bIsInDanger = false;

	if (NearestZombie)
	{
		const float DistSq =
			FVector::DistSquared(Pawn->GetActorLocation(), NearestZombie->GetActorLocation());

		bIsInDanger = DistSq < FMath::Square(750.f);
	}

	BB->SetValueAsBool(TEXT("IsInDanger"), bIsInDanger);
}

AActor* UUpdateSurvivorWorldState::FindNearestZombie(APawn* SurvivorPawn) const
{
	if (!SurvivorPawn)
		return nullptr;

	TArray<AActor*> Zombies;
	UGameplayStatics::GetAllActorsOfClass(
		SurvivorPawn->GetWorld(),
		ABaseZombie::StaticClass(),
		Zombies);

	AActor* BestZombie = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	const FVector SurvivorLocation = SurvivorPawn->GetActorLocation();

	for (AActor* Zombie : Zombies)
	{
		if (!Zombie)
			continue;

		const float DistSq = FVector::DistSquared(SurvivorLocation, Zombie->GetActorLocation());

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestZombie = Zombie;
		}
	}

	return BestZombie;
}

AActor* UUpdateSurvivorWorldState::FindBestItem(APawn* SurvivorPawn) const
{
	if (!SurvivorPawn)
		return nullptr;

	TArray<AActor*> Items;
	UGameplayStatics::GetAllActorsOfClass(
		SurvivorPawn->GetWorld(),
		ABaseItem::StaticClass(),
		Items);

	AActor* BestItem = nullptr;
	float BestScore = -FLT_MAX;

	const FVector SurvivorLocation = SurvivorPawn->GetActorLocation();

	for (AActor* ItemActor : Items)
	{
		if (!ItemActor)
			continue;

		ABaseItem* Item = Cast<ABaseItem>(ItemActor);
		if (!Item)
			continue;

		const float Distance = FVector::Dist(SurvivorLocation, Item->GetActorLocation());

		float Score = 1000.f / FMath::Max(Distance, 1.f);

		// Basic item priority
		const FString ItemName = Item->GetName();

		if (ItemName.Contains(TEXT("Pistol")) || ItemName.Contains(TEXT("Shotgun")))
		{
			Score += 100.f;
		}
		else if (ItemName.Contains(TEXT("Medkit")))
		{
			Score += 80.f;
		}
		else if (ItemName.Contains(TEXT("Food")))
		{
			Score += 40.f;
		}
		else if (ItemName.Contains(TEXT("Garbage")))
		{
			Score -= 1000.f;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestItem = Item;
		}
	}

	return BestItem;
}

bool UUpdateSurvivorWorldState::HasUsefulWeapon(APawn* SurvivorPawn) const
{
	if (!SurvivorPawn)
		return false;

	UInventoryComponent* Inventory = SurvivorPawn->GetComponentByClass<UInventoryComponent>();
	if (!Inventory)
		return false;

	
	for (AActor* Item : Inventory->GetInventory())
	{
		if (!Item)
			continue;

		const FString Name = Item->GetName();

		if (Name.Contains(TEXT("Pistol")) || Name.Contains(TEXT("Shotgun")))
		{
			return true;
		}
	}

	return false;
}

bool UUpdateSurvivorWorldState::HasItemType(APawn* SurvivorPawn, const FString& ItemTypeName) const
{
	if (!SurvivorPawn)
		return false;

	UInventoryComponent* Inventory = SurvivorPawn->GetComponentByClass<UInventoryComponent>();
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