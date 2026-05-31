#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "StudentPerceptor.generated.h"

class ABaseZombie;
class ABaseItem;

UENUM(BlueprintType)
enum class EStudentMemoryType : uint8
{
	Unknown,
	Zombie,
	Weapon,
	Medkit,
	Food,
	Garbage,
	House
};

USTRUCT(BlueprintType)
struct FStudentMemoryEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Actor;

	UPROPERTY(BlueprintReadOnly)
	FVector LastKnownLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	EStudentMemoryType Type = EStudentMemoryType::Unknown;

	UPROPERTY(BlueprintReadOnly)
	float LastSeenTime = 0.f;

	UPROPERTY(BlueprintReadOnly)
	bool bCurrentlyVisible = false;

	UPROPERTY(BlueprintReadOnly)
	bool bVisited = false;

	UPROPERTY(BlueprintReadOnly)
	float AvoidUntilTime = 0.f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KALRAKARANZOMBIERUNTIME_API UStudentPerceptor : public UActorComponent
{
	GENERATED_BODY()

public:
	UStudentPerceptor();

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	const TArray<FStudentMemoryEntry>& GetMemory() const { return Memory; }

	AActor* GetNearestKnownZombie(const FVector& FromLocation, bool bOnlyVisible = false) const;

	AActor* GetBestKnownItem(
		const FVector& FromLocation,
		bool bNeedsHealing,
		bool bNeedsStamina,
		bool bHasWeapon) const;

	AActor* GetBestKnownWeapon(const FVector& FromLocation) const;
	AActor* GetBestKnownMedkit(const FVector& FromLocation) const;
	AActor* GetBestKnownFood(const FVector& FromLocation) const;
	AActor* GetBestKnownHouse(const FVector& FromLocation) const;

	AActor* GetBestUnvisitedHouse(const FVector& FromLocation) const;

	void MarkHouseVisited(AActor* HouseActor);
	void MarkHouseVisitedNearLocation(const FVector& Location, float Radius);

	void AvoidActorForTime(AActor* Actor, float Duration);
	bool IsMemoryAvoided(const FStudentMemoryEntry& Entry) const;

	void IgnoreActorForTime(AActor* Actor, float Duration);

	bool GetBestKnownLocationOfType(
		EStudentMemoryType Type,
		const FVector& FromLocation,
		FVector& OutLocation) const;

	int32 GetKnownZombieCount(float MaxDistance, const FVector& FromLocation) const;

protected:
	UPROPERTY()
	UAIPerceptionComponent* PerceptionComponent = nullptr;

	UPROPERTY()
	TArray<FStudentMemoryEntry> Memory;

	UPROPERTY(EditAnywhere, Category = "Student AI Debug")
	bool bDebugMessages = true;

private:
	void AddOrUpdateMemory(AActor* Actor, EStudentMemoryType Type, bool bVisible);
	void MarkActorNotVisible(AActor* Actor);
	void RemoveInvalidMemory();

	int32 FindMemoryIndex(AActor* Actor) const;

	EStudentMemoryType ClassifyActor(AActor* Actor) const;

	bool IsZombie(AActor* Actor) const;
	bool IsItem(AActor* Actor) const;
	bool IsHouse(AActor* Actor) const;

	float ScoreItem(
		AActor* ItemActor,
		const FVector& FromLocation,
		bool bNeedsHealing,
		bool bNeedsStamina,
		bool bHasWeapon) const;

	float DistanceScore(const FVector& FromLocation, const FVector& TargetLocation) const;

	void DebugMessage(const FString& Message, const FColor& Color) const;
};