#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindExploreLocation.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_FindExploreLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindExploreLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetLocationKey = TEXT("TargetLocation");

	UPROPERTY(EditAnywhere, Category = "Explore")
	float ExploreRadius = 1200.f;
};