#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PickupItem.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_PickupItem : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PickupItem();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetItemKey = TEXT("TargetItem");
};