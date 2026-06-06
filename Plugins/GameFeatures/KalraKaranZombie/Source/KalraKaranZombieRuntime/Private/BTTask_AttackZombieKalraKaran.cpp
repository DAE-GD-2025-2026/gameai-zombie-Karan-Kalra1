#include "BTTask_AttackZombieKalraKaran.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"

UBTTask_AttackZombieKalraKaran::UBTTask_AttackZombieKalraKaran()
{
	NodeName = TEXT("Attack Zombie");
}

EBTNodeResult::Type UBTTask_AttackZombieKalraKaran::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.f,
			FColor::Red,
			TEXT("AttackZombie task is running")
		);
	}

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

	UInventoryComponent* Inventory =
		Pawn->GetComponentByClass<UInventoryComponent>();

	if (!Inventory)
		return EBTNodeResult::Failed;

	// Face zombie first
	FVector Direction = Zombie->GetActorLocation() - Pawn->GetActorLocation();
	Direction.Z = 0.f;

	if (!Direction.IsNearlyZero())
	{
		FRotator LookRotation = Direction.Rotation();
		Pawn->SetActorRotation(LookRotation);
	}

	const TArray<ABaseItem*>& Items = Inventory->GetInventory();

	int WeaponSlot = INDEX_NONE;

	// Prefer pistol because it does higher direct damage
	for (int i = 0; i < Items.Num(); ++i)
	{
		if (Items[i] && Items[i]->GetItemType() == EItemType::Pistol && Items[i]->GetValue() > 0)
		{
			WeaponSlot = i;
			break;
		}
	}

	// If no pistol, use shotgun
	if (WeaponSlot == INDEX_NONE)
	{
		for (int i = 0; i < Items.Num(); ++i)
		{
			if (Items[i] && Items[i]->GetItemType() == EItemType::Shotgun && Items[i]->GetValue() > 0)
			{
				WeaponSlot = i;
				break;
			}
		}
	}

	if (WeaponSlot == INDEX_NONE)
		return EBTNodeResult::Failed;

	if (Inventory->UseItem(WeaponSlot))
	{
		if (Items[WeaponSlot] && Items[WeaponSlot]->GetValue() <= 0)
		{
			Inventory->RemoveItem(WeaponSlot);
		}

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}