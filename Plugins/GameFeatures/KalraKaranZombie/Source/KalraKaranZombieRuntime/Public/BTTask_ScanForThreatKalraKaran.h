// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ScanForThreatKalraKaran.generated.h"

/**
 * 
 */
UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTTask_ScanForThreatKalraKaran : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ScanForThreatKalraKaran();


protected:

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	virtual void TickTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds) override;

	virtual uint16 GetInstanceMemorySize() const override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetZombieKey = TEXT("TargetZombie");

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName IsDamageScanningKey = TEXT("IsDamageScanning");

	UPROPERTY(EditAnywhere, Category = "Scan")
	float ScanDuration = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Scan")
	float RotationSpeedDegreesPerSecond = 360.f;
};

struct FSpinScanMemory
{
	float ElapsedTime = 0.f;
};