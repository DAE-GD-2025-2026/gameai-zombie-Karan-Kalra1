#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UseFoodKalraKaran.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_UseFoodKalraKaran : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UseFoodKalraKaran();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;
};