#include "BTTask_PickupItem.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"

UBTTask_PickupItem::UBTTask_PickupItem()
{
	NodeName = TEXT("Pickup Item");
}

EBTNodeResult::Type UBTTask_PickupItem::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
		return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	ABaseItem* Item = Cast<ABaseItem>(BB->GetValueAsObject(TargetItemKey));
	if (!Item)
		return EBTNodeResult::Failed;

	APawn* Pawn = AIController->GetPawn();

	UInventoryComponent* Inventory =
		Pawn->GetComponentByClass<UInventoryComponent>();

	if (!Inventory)
		return EBTNodeResult::Failed;

	const float DistSq =
		FVector::DistSquared(Pawn->GetActorLocation(), Item->GetActorLocation());

	if (DistSq > FMath::Square(Inventory->GetPickupRange()))
	{
		return EBTNodeResult::Failed;
	}

	const TArray<ABaseItem*>& Items = Inventory->GetInventory();

	for (int i = 0; i < Inventory->GetInventoryCapacity(); ++i)
	{
		if (!Items[i])
		{
			if (Inventory->GrabItem(i, Item))
			{
				BB->ClearValue(TargetItemKey);
				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}