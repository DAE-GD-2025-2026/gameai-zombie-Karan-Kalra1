#include "BTTask_UseMedkitKalraKaran.h"

#include "AIController.h"

#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"

UBTTask_UseMedkitKalraKaran::UBTTask_UseMedkitKalraKaran()
{
	NodeName = TEXT("Use Medkit");
}

EBTNodeResult::Type UBTTask_UseMedkitKalraKaran::ExecuteTask(
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

	for (int i = 0; i < Items.Num(); ++i)
	{
		ABaseItem* Item = Items[i];

		if (Item && Item->GetItemType() == EItemType::Medkit)
		{
			if (Inventory->UseItem(i))
			{
				if (Item->GetValue() <= 0)
				{
					Inventory->RemoveItem(i);
				}

				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}