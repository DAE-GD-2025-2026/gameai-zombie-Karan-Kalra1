#include "BTTask_CleanGarbageKalraKaran.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Common/InventoryComponent.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"
#include "../StudentPerceptorKalraKaran.h"

UBTTask_CleanGarbageKalraKaran::UBTTask_CleanGarbageKalraKaran()
{
	NodeName = TEXT("Clean Garbage Kalra Karan");
}

EBTNodeResult::Type UBTTask_CleanGarbageKalraKaran::ExecuteTask(
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

	ABaseItem* Garbage = Cast<ABaseItem>(BB->GetValueAsObject(TargetGarbageKey));
	if (!Garbage || Garbage->IsPendingKillPending())
	{
		BB->ClearValue(TargetGarbageKey);
		return EBTNodeResult::Failed;
	}

	if (Garbage->GetItemType() != EItemType::Garbage)
	{
		BB->ClearValue(TargetGarbageKey);
		return EBTNodeResult::Failed;
	}

	UInventoryComponent* Inventory =
		Pawn->GetComponentByClass<UInventoryComponent>();

	if (!Inventory)
		return EBTNodeResult::Failed;

	const float PickupRange = Inventory->GetPickupRange() + 50.f;

	const float Distance = FVector::Dist(
		Pawn->GetActorLocation(),
		Garbage->GetActorLocation());

	if (Distance > PickupRange)
	{
		return EBTNodeResult::Failed;
	}

	const TArray<ABaseItem*>& Items = Inventory->GetInventory();

	for (int32 SlotIdx = 0; SlotIdx < Inventory->GetInventoryCapacity(); ++SlotIdx)
	{
		if (SlotIdx >= Items.Num())
			break;

		if (!Items[SlotIdx])
		{
			if (Inventory->GrabItem(SlotIdx, Garbage))
			{
				Inventory->RemoveItem(SlotIdx);

				BB->ClearValue(TargetGarbageKey);

				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						1.f,
						FColor::Orange,
						TEXT("Garbage cleaned")
					);
				}

				return EBTNodeResult::Succeeded;
			}
		}
	}

	UStudentPerceptorKalraKaran* Perceptor =
		AIController->GetComponentByClass<UStudentPerceptorKalraKaran>();

	if (!Perceptor)
	{
		Perceptor = Pawn->GetComponentByClass<UStudentPerceptorKalraKaran>();
	}

	if (Perceptor)
	{
		Perceptor->IgnoreActorForTime(Garbage, 30.f);
	}

	BB->ClearValue(TargetGarbageKey);

	return EBTNodeResult::Failed;
}