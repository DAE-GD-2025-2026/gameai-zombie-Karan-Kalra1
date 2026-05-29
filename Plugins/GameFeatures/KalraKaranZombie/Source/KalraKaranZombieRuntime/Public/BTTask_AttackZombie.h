#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AttackZombie.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_AttackZombie : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_AttackZombie();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetZombieKey = TEXT("TargetZombie");
};