#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveAsideKaranKalra.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_MoveAsideKaranKalra : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveAsideKaranKalra();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetZombieKey = TEXT("TargetZombie");

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName SafeLocationKey = TEXT("SafeLocation");

	UPROPERTY(EditAnywhere, Category = "Move Aside")
	float StepDistance = 700.f;

	UPROPERTY(EditAnywhere, Category = "Move Aside")
	float NavProjectionExtent = 500.f;
};