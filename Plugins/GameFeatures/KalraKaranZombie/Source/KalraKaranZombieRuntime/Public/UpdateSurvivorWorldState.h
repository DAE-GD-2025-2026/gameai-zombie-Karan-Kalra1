#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UpdateSurvivorWorldState.generated.h"

class UStudentPerceptor;

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UUpdateSurvivorWorldState : public UBTService
{
	GENERATED_BODY()

public:
	UUpdateSurvivorWorldState();

protected:
	virtual void TickNode(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds) override;

private:
	UStudentPerceptor* GetPerceptor(AAIController* AIController, APawn* Pawn) const;

	bool HasUsefulWeapon(APawn* SurvivorPawn) const;
	bool HasItemType(APawn* SurvivorPawn, const FString& ItemTypeName) const;
};