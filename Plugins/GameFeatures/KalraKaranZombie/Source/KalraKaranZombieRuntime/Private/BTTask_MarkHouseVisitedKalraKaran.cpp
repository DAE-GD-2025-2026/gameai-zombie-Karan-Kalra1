#include "BTTask_MarkHouseVisitedKalraKaran.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../StudentPerceptorKalraKaran.h"

UBTTask_MarkHouseVisitedKalraKaran::UBTTask_MarkHouseVisitedKalraKaran()
{
	NodeName = TEXT("Mark House Visited");
}

EBTNodeResult::Type UBTTask_MarkHouseVisitedKalraKaran::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
		return EBTNodeResult::Failed;

	AActor* House = Cast<AActor>(BB->GetValueAsObject(TargetHouseKey));
	if (!House)
		return EBTNodeResult::Failed;

	UStudentPerceptorKalraKaran* Perceptor =
		AIController->GetComponentByClass<UStudentPerceptorKalraKaran>();

	if (!Perceptor && AIController->GetPawn())
	{
		Perceptor = AIController->GetPawn()->GetComponentByClass<UStudentPerceptorKalraKaran>();
	}

	if (!Perceptor)
		return EBTNodeResult::Failed;

	Perceptor->MarkHouseVisited(House);

	BB->ClearValue(TargetHouseKey);

	return EBTNodeResult::Succeeded;
}