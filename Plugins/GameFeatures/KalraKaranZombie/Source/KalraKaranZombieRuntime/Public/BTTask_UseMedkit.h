#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UseMedkit.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_UseMedkit : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UseMedkit();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;
};