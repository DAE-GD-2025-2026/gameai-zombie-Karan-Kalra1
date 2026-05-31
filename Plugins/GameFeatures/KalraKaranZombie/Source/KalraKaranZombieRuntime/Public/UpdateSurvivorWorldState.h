#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UpdateSurvivorWorldState.generated.h"


class UStudentPerceptor;
class UInventoryComponent;

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

	AActor* FindNearestPurgeZone(APawn* SurvivorPawn, float& OutDistance) const;
	bool IsInventoryFull(APawn* SurvivorPawn) const;

	float GetHealthValue(APawn* SurvivorPawn) const;
	float GetStaminaValue(APawn* SurvivorPawn) const;

	float CalculateThreatLevel(
		APawn* SurvivorPawn,
		AActor* NearestZombie,
		int32 NearbyZombieCount,
		bool bNeedsHealing,
		bool bHasWeapon) const;

private:
	// Stuck detection
	bool bHasLastPawnLocation = false;
	FVector LastPawnLocation = FVector::ZeroVector;
	float TimeSinceMoved = 0.f;
};