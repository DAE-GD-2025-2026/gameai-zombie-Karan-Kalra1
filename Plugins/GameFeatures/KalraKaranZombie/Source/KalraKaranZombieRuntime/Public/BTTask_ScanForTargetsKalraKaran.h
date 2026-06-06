#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ScanForTargetsKalraKaran.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_ScanForTargetsKalraKaran : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ScanForTargetsKalraKaran();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Scan")
	float MinRotationAmount = 90.f;

	UPROPERTY(EditAnywhere, Category = "Scan")
	float MaxRotationAmount = 180.f;
};