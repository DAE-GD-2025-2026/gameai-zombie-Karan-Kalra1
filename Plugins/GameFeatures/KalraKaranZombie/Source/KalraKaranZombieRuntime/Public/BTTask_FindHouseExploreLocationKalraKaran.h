#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindHouseExploreLocationKalraKaran.generated.h"

class UStudentPerceptorKalraKaran;

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_FindHouseExploreLocationKalraKaran : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindHouseExploreLocationKalraKaran();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetLocationKey = TEXT("TargetLocation");

	UPROPERTY(EditAnywhere, Category = "Explore")
	float ExploreRadius = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Explore")
	float HouseReachRadius = 250.f;

	UPROPERTY(EditAnywhere, Category = "Explore")
	float NavProjectionExtent = 500.f;

private:
	UStudentPerceptorKalraKaran* GetPerceptor(AAIController* AIController, APawn* Pawn) const;

	bool TryUseKnownHouse(
		UBehaviorTreeComponent& OwnerComp,
		APawn* Pawn,
		UStudentPerceptorKalraKaran* Perceptor) const;

	bool TryUseRandomReachablePoint(
		UBehaviorTreeComponent& OwnerComp,
		APawn* Pawn) const;
};