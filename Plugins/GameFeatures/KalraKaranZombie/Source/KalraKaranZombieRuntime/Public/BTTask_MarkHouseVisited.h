#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MarkHouseVisited.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_MarkHouseVisited : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MarkHouseVisited();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetHouseKey = TEXT("TargetHouse");
};