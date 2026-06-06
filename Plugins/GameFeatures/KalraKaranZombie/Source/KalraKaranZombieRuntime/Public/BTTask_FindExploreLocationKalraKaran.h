#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindExploreLocationKalraKaran.generated.h"


class UStudentPerceptorKalraKaran;

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_FindExploreLocationKalraKaran : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindExploreLocationKalraKaran();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetLocationKey = TEXT("TargetLocation");

	UPROPERTY(EditAnywhere, Category = "Explore")
	float ExploreRadius = 1200.f;
};