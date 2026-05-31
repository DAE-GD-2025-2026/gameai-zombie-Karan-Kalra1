#include "BTTask_UseFood.h"

#include "AIController.h"

#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"

UBTTask_UseFood::UBTTask_UseFood()
{
	NodeName = TEXT("Use Food");
}

EBTNodeResult::Type UBTTask_UseFood::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
		return EBTNodeResult::Failed;

	APawn* Pawn = AIController->GetPawn();

	UInventoryComponent* Inventory =
		Pawn->GetComponentByClass<UInventoryComponent>();

	if (!Inventory)
		return EBTNodeResult::Failed;

	const TArray<ABaseItem*>& Items = Inventory->GetInventory();

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		ABaseItem* Item = Items[i];

		if (Item && Item->GetItemType() == EItemType::Food)
		{
			if (Inventory->UseItem(i))
			{
				if (Item->GetValue() <= 0)
				{
					Inventory->RemoveItem(i);
				}

				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						1.f,
						FColor::Green,
						TEXT("Used food for stamina")
					);
				}

				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}