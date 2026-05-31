#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindPurgeEscapeLocation.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_FindPurgeEscapeLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindPurgeEscapeLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName PurgeZoneLocationKey = TEXT("PurgeZoneLocation");

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName SafeLocationKey = TEXT("SafeLocation");

	UPROPERTY(EditAnywhere, Category = "Purge")
	float EscapeDistance = 1800.f;

	UPROPERTY(EditAnywhere, Category = "Purge")
	float NavProjectionExtent = 600.f;
};