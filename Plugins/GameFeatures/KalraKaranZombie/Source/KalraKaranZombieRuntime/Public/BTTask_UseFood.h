#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UseFood.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_UseFood : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UseFood();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;
};