#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h" 
#include "Perception/AISenseConfig_Damage.h" 
#include "Perception/AISense_Damage.h" 
#include "StudentPerceptor.generated.h"

class ABaseZombie;
class ABaseItem;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KALRAKARANZOMBIERUNTIME_API UStudentPerceptor : public UActorComponent
{
	GENERATED_BODY()

public:
	UStudentPerceptor();

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	const TArray<TWeakObjectPtr<AActor>>& GetKnownZombies() const { return KnownZombies; }
	const TArray<TWeakObjectPtr<AActor>>& GetKnownItems() const { return KnownItems; }

	AActor* GetNearestKnownZombie(const FVector& FromLocation) const;
	AActor* GetBestKnownItem(const FVector& FromLocation) const;

private:
	UPROPERTY()
	UAIPerceptionComponent* PerceptionComponent = nullptr;

	TArray<TWeakObjectPtr<AActor>> KnownZombies;
	TArray<TWeakObjectPtr<AActor>> KnownItems;

	void AddKnownActor(AActor* Actor);
	void RemoveKnownActor(AActor* Actor);

	void RemoveInvalidEntries();

	bool IsZombie(AActor* Actor) const;
	bool IsItem(AActor* Actor) const;
};