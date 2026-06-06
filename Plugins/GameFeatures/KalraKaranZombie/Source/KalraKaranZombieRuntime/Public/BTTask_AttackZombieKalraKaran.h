#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AttackZombieKalraKaran.generated.h"

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_AttackZombieKalraKaran : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_AttackZombieKalraKaran();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetZombieKey = TEXT("TargetZombie");
};