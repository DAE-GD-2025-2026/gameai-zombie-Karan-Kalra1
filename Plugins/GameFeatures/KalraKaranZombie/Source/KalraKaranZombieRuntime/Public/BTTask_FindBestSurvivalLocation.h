#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindBestSurvivalLocation.generated.h"

class UStudentPerceptor;

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_FindBestSurvivalLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindBestSurvivalLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetZombieKey = TEXT("TargetZombie");

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetItemKey = TEXT("TargetItem");

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName SafeLocationKey = TEXT("SafeLocation");

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName NeedsHealingKey = TEXT("NeedsHealing");

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName HasWeaponKey = TEXT("HasWeapon");

	UPROPERTY(EditAnywhere, Category = "Flee")
	float FleeDistance = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Flee")
	float MinMoveDistance = 250.f;

	UPROPERTY(EditAnywhere, Category = "Flee")
	float NavProjectionExtent = 500.f;

private:
	UStudentPerceptor* GetPerceptor(AAIController* AIController, APawn* Pawn) const;

	bool TryUseActorLocation(
		UBehaviorTreeComponent& OwnerComp,
		APawn* Pawn,
		AActor* Actor,
		FName LocationKey,
		FName OptionalTargetActorKey = NAME_None) const;

	bool TryFindPointAwayFromZombie(
		UBehaviorTreeComponent& OwnerComp,
		APawn* Pawn,
		AActor* Zombie) const;

	bool TryFindRandomFallback(
		UBehaviorTreeComponent& OwnerComp,
		APawn* Pawn) const;
};