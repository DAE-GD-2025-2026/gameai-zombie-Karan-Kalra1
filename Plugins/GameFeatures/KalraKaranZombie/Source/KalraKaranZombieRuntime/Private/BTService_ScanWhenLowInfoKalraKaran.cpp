#include "BTService_ScanWhenLowInfoKalraKaran.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "../StudentPerceptorKalraKaran.h"

UBTService_ScanWhenLowInfoKalraKaran::UBTService_ScanWhenLowInfoKalraKaran()
{
	NodeName = TEXT("Scan When Low Info");

	// Runs often enough to scan while fleeing/exploring.
	Interval = 0.6f;
	RandomDeviation = 0.15f;
}

void UBTService_ScanWhenLowInfoKalraKaran::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return;

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
		return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return;

	UStudentPerceptorKalraKaran* Perceptor = GetPerceptor(AIController, Pawn);

	const bool bHasUsefulKnowledge =
		HasUsefulKnowledge(OwnerComp, Perceptor, Pawn);

	const bool bShouldScan = !bHasUsefulKnowledge;

	BB->SetValueAsBool(TEXT("ShouldScan"), bShouldScan);

	if (!bShouldScan)
		return;

	RotatePawn(Pawn);

	if (bDebugScan && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			0.5f,
			FColor::Cyan,
			TEXT("Low-info scan")
		);
	}
}

UStudentPerceptorKalraKaran* UBTService_ScanWhenLowInfoKalraKaran::GetPerceptor(
	AAIController* AIController,
	APawn* Pawn) const
{
	if (AIController)
	{
		if (UStudentPerceptorKalraKaran* Perceptor =
			AIController->GetComponentByClass<UStudentPerceptorKalraKaran>())
		{
			return Perceptor;
		}
	}

	if (Pawn)
	{
		if (UStudentPerceptorKalraKaran* Perceptor =
			Pawn->GetComponentByClass<UStudentPerceptorKalraKaran>())
		{
			return Perceptor;
		}
	}

	return nullptr;
}

bool UBTService_ScanWhenLowInfoKalraKaran::HasUsefulKnowledge(
	UBehaviorTreeComponent& OwnerComp,
	UStudentPerceptorKalraKaran* Perceptor,
	APawn* Pawn) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB || !Pawn)
		return false;

	// If a current target item exists, we have useful info.
	if (BB->GetValueAsObject(TEXT("TargetItem")))
		return true;

	const FVector PawnLocation = Pawn->GetActorLocation();

	const bool bNeedsHealing = BB->GetValueAsBool(TEXT("NeedsHealing"));
	const bool bHasWeapon = BB->GetValueAsBool(TEXT("HasWeapon"));

	if (!Perceptor)
		return false;

	// If hurt, medkit knowledge is very useful.
	if (bNeedsHealing && Perceptor->GetBestKnownMedkit(PawnLocation))
		return true;

	// If unarmed, weapon knowledge is very useful.
	if (!bHasWeapon && Perceptor->GetBestKnownWeapon(PawnLocation))
		return true;

	// Any unvisited house is useful for exploration.
	if (Perceptor->GetBestUnvisitedHouse(PawnLocation))
		return true;

	// Food is useful too, but lower priority.
	if (Perceptor->GetBestKnownFood(PawnLocation))
		return true;

	return false;
}

void UBTService_ScanWhenLowInfoKalraKaran::RotatePawn(APawn* Pawn) const
{
	if (!Pawn)
		return;

	const float YawAmount =
		FMath::RandRange(MinYawRotation, MaxYawRotation);

	const float Direction =
		FMath::RandBool() ? 1.f : -1.f;

	FRotator NewRotation = Pawn->GetActorRotation();
	NewRotation.Yaw += YawAmount * Direction;

	Pawn->SetActorRotation(NewRotation);
}