#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_UseMedkitKalraKaran.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_UseMedkitKalraKaran : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UseMedkitKalraKaran();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;
};