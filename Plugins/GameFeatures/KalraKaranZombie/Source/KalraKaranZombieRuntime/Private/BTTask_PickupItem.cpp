#include "BTTask_PickupItem.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "../StudentPerceptor.h"

UBTTask_PickupItem::UBTTask_PickupItem()
{
	NodeName = TEXT("Pickup Item");
}

EBTNodeResult::Type UBTTask_PickupItem::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.f,
			FColor::Cyan,
			TEXT("PickupItem task is running")
		);
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
		return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	ABaseItem* Item = Cast<ABaseItem>(BB->GetValueAsObject(TargetItemKey));
	if (!Item || Item->IsPendingKillPending())
	{
		BB->ClearValue(TargetItemKey);
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AIController->GetPawn();

	UInventoryComponent* Inventory =
		Pawn->GetComponentByClass<UInventoryComponent>();

	if (!Inventory)
		return EBTNodeResult::Failed;

	const float PickupRange = Inventory->GetPickupRange() + 25.f;

	const float Distance =
		FVector::Dist(Pawn->GetActorLocation(), Item->GetActorLocation());



	if (Distance > PickupRange)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				1.f,
				FColor::Yellow,
				FString::Printf(TEXT("Pickup failed: too far %.1f / %.1f"), Distance, PickupRange)
			);
		}

		return EBTNodeResult::Failed;
	}

	const TArray<ABaseItem*>& Items = Inventory->GetInventory();

	for (int i = 0; i < Inventory->GetInventoryCapacity(); ++i)
	{
		if (i >= Items.Num())
			break;

		if (!Items[i])
		{
			if (Inventory->GrabItem(i, Item))
			{
				BB->ClearValue(TargetItemKey);
				return EBTNodeResult::Succeeded;
			}
		}
	}

	UStudentPerceptor* Perceptor =
		AIController->GetComponentByClass<UStudentPerceptor>();

	if (!Perceptor && Pawn)
	{
		Perceptor = Pawn->GetComponentByClass<UStudentPerceptor>();
	}

	if (Perceptor)
	{
		Perceptor->IgnoreActorForTime(Item, 15.f);
	}

	BB->ClearValue(TargetItemKey);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.f,
			FColor::Red,
			TEXT("Pickup failed: inventory full or GrabItem failed")
		);
	}

	return EBTNodeResult::Failed;
}