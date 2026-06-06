#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MarkHouseVisitedKalraKaran.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_MarkHouseVisitedKalraKaran : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MarkHouseVisitedKalraKaran();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetHouseKey = TEXT("TargetHouse");
};