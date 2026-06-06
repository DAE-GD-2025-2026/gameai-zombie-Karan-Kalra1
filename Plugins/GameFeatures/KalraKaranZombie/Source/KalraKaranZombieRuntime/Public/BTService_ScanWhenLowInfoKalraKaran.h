#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_ScanWhenLowInfoKalraKaran.generated.h"

class UStudentPerceptorKalraKaran;

UCLASS()
class KALRAKARANZOMBIERUNTIME_API UBTService_ScanWhenLowInfoKalraKaran : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_ScanWhenLowInfoKalraKaran();

protected:
	virtual void TickNode(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Scan")
	float MinYawRotation = 60.f;

	UPROPERTY(EditAnywhere, Category = "Scan")
	float MaxYawRotation = 140.f;

	UPROPERTY(EditAnywhere, Category = "Scan")
	bool bDebugScan = false;

private:
	UStudentPerceptorKalraKaran* GetPerceptor(AAIController* AIController, APawn* Pawn) const;

	bool HasUsefulKnowledge(
		UBehaviorTreeComponent& OwnerComp,
		UStudentPerceptorKalraKaran* Perceptor,
		APawn* Pawn) const;

	void RotatePawn(APawn* Pawn) const;
};