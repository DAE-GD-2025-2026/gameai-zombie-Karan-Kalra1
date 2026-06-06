// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ScanForThreatKalraKaran.h"


#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_ScanForThreatKalraKaran::UBTTask_ScanForThreatKalraKaran()
{
	NodeName = TEXT("Spin Scan For Threat");

	bNotifyTick = true;
}

uint16 UBTTask_ScanForThreatKalraKaran::GetInstanceMemorySize() const
{
	return sizeof(FSpinScanMemory);
}

EBTNodeResult::Type UBTTask_ScanForThreatKalraKaran::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	FSpinScanMemory* Memory = reinterpret_cast<FSpinScanMemory*>(NodeMemory);
	Memory->ElapsedTime = 0.f;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsBool(IsDamageScanningKey, true);

		// If perception already found the zombie, no need to scan.
		if (BB->GetValueAsObject(TargetZombieKey))
		{
			BB->SetValueAsBool(IsDamageScanningKey, false);
			return EBTNodeResult::Succeeded;
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			1.f,
			FColor::Orange,
			TEXT("Spin scanning for attacker")
		);
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_ScanForThreatKalraKaran::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// If perception found a zombie while spinning, stop immediately.
	if (BB->GetValueAsObject(TargetZombieKey))
	{
		BB->SetValueAsBool(IsDamageScanningKey, false);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	APawn* Pawn = AIController->GetPawn();

	FRotator NewRotation = Pawn->GetActorRotation();
	NewRotation.Yaw += RotationSpeedDegreesPerSecond * DeltaSeconds;
	Pawn->SetActorRotation(NewRotation);

	FSpinScanMemory* Memory = reinterpret_cast<FSpinScanMemory*>(NodeMemory);
	Memory->ElapsedTime += DeltaSeconds;

	if (Memory->ElapsedTime >= ScanDuration)
	{
		BB->SetValueAsBool(IsDamageScanningKey, false);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}