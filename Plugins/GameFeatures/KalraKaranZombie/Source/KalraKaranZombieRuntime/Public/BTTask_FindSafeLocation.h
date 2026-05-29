#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindSafeLocation.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_FindSafeLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindSafeLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetZombieKey = TEXT("TargetZombie");

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName SafeLocationKey = TEXT("SafeLocation");

	UPROPERTY(EditAnywhere, Category = "Flee")
	float FleeDistance = 1000.f;
};