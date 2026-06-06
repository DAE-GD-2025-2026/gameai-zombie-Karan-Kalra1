#include "BTTask_ScanForTargetsKalraKaran.h"

#include "AIController.h"

UBTTask_ScanForTargetsKalraKaran::UBTTask_ScanForTargetsKalraKaran()
{
	NodeName = TEXT("Scan For Targets");
}

EBTNodeResult::Type UBTTask_ScanForTargetsKalraKaran::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController || !AIController->GetPawn())
		return EBTNodeResult::Failed;

	APawn* Pawn = AIController->GetPawn();

	const float RandomRotation =
		FMath::RandRange(MinRotationAmount, MaxRotationAmount);

	const float DirectionSign = FMath::RandBool() ? 1.f : -1.f;

	FRotator NewRotation = Pawn->GetActorRotation();
	NewRotation.Yaw += RandomRotation * DirectionSign;

	Pawn->SetActorRotation(NewRotation);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			0.5f,
			FColor::Cyan,
			TEXT("Scanning aggressively")
		);
	}

	return EBTNodeResult::Succeeded;
}