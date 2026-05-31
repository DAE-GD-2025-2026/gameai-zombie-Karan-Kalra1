#include "BTService_ScanWhenLowInfo.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "../StudentPerceptor.h"

UBTService_ScanWhenLowInfo::UBTService_ScanWhenLowInfo()
{
	NodeName = TEXT("Scan When Low Info");

	// Runs often enough to scan while fleeing/exploring.
	Interval = 0.6f;
	RandomDeviation = 0.15f;
}

void UBTService_ScanWhenLowInfo::TickNode(
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

	UStudentPerceptor* Perceptor = GetPerceptor(AIController, Pawn);

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

UStudentPerceptor* UBTService_ScanWhenLowInfo::GetPerceptor(
	AAIController* AIController,
	APawn* Pawn) const
{
	if (AIController)
	{
		if (UStudentPerceptor* Perceptor =
			AIController->GetComponentByClass<UStudentPerceptor>())
		{
			return Perceptor;
		}
	}

	if (Pawn)
	{
		if (UStudentPerceptor* Perceptor =
			Pawn->GetComponentByClass<UStudentPerceptor>())
		{
			return Perceptor;
		}
	}

	return nullptr;
}

bool UBTService_ScanWhenLowInfo::HasUsefulKnowledge(
	UBehaviorTreeComponent& OwnerComp,
	UStudentPerceptor* Perceptor,
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

void UBTService_ScanWhenLowInfo::RotatePawn(APawn* Pawn) const
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