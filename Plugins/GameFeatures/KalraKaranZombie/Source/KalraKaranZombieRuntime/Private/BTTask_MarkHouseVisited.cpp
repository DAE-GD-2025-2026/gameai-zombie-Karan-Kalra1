#include "BTTask_MarkHouseVisited.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../StudentPerceptor.h"

UBTTask_MarkHouseVisited::UBTTask_MarkHouseVisited()
{
	NodeName = TEXT("Mark House Visited");
}

EBTNodeResult::Type UBTTask_MarkHouseVisited::ExecuteTask(
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

	UStudentPerceptor* Perceptor =
		AIController->GetComponentByClass<UStudentPerceptor>();

	if (!Perceptor && AIController->GetPawn())
	{
		Perceptor = AIController->GetPawn()->GetComponentByClass<UStudentPerceptor>();
	}

	if (!Perceptor)
		return EBTNodeResult::Failed;

	Perceptor->MarkHouseVisited(House);

	BB->ClearValue(TargetHouseKey);

	return EBTNodeResult::Succeeded;
}