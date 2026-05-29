// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UpdateSurvivorWorldState.generated.h"

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
	AActor* FindNearestZombie(APawn* SurvivorPawn) const;
	AActor* FindBestItem(APawn* SurvivorPawn) const;

	bool HasUsefulWeapon(APawn* SurvivorPawn) const;
	bool HasItemType(APawn* SurvivorPawn, const FString& ItemTypeName) const;
};