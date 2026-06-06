#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UpdateSurvivorWorldStateKalraKaran.generated.h"


class UStudentPerceptorKalraKaran;
class UInventoryComponent;

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UUpdateSurvivorWorldStateKalraKaran : public UBTService
{
	GENERATED_BODY()

public:
	UUpdateSurvivorWorldStateKalraKaran();

protected:
	virtual void TickNode(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds) override;

private:
	UStudentPerceptorKalraKaran* GetPerceptor(AAIController* AIController, APawn* Pawn) const;

	bool HasUsefulWeapon(APawn* SurvivorPawn) const;
	bool HasItemType(APawn* SurvivorPawn, const FString& ItemTypeName) const;

	//AActor* FindNearestPurgeZone(APawn* SurvivorPawn, float& OutDistance) const;

	bool IsInventoryFull(APawn* SurvivorPawn) const;

	float GetHealthValue(APawn* SurvivorPawn) const;
	float GetStaminaValue(APawn* SurvivorPawn) const;

	float CalculateThreatLevel(
		APawn* SurvivorPawn,
		AActor* NearestZombie,
		int32 NearbyZombieCount,
		bool bNeedsHealing,
		bool bHasWeapon) const;

	// Stuck detection
	bool bHasLastPawnLocation = false;
	FVector LastPawnLocation = FVector::ZeroVector;
	float TimeSinceMoved = 0.f;

	bool bHasPreviousHealth = false;
	float PreviousHealthValue = 0.f;
	float RecentlyDamagedTimer = 0.f;


};