#include "StudentPerceptorKalraKaran.h"

#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

#include "Zombies/BaseZombie.h"
#include "Items/BaseItem.h"
#include "Items/ItemType.h"

UStudentPerceptorKalraKaran::UStudentPerceptorKalraKaran()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStudentPerceptorKalraKaran::BeginPlay()
{
	Super::BeginPlay();

	PerceptionComponent = GetOwner()->GetComponentByClass<UAIPerceptionComponent>();

	if (PerceptionComponent)
	{
		PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
			this,
			&UStudentPerceptorKalraKaran::OnPerceptionUpdated);

		DebugMessage(TEXT("StudentPerceptor connected to AIPerception"), FColor::Green);
	}
	else
	{
		DebugMessage(TEXT("StudentPerceptor: No AIPerceptionComponent found on owner"), FColor::Red);
	}
}

void UStudentPerceptorKalraKaran::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor)
		return;

	RemoveInvalidMemory();

	const EStudentMemoryType Type = ClassifyActor(Actor);

	const bool bWasSeen = Stimulus.WasSuccessfullySensed();
	const bool bIsDamageStimulus =
		Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>();

	// If the actor damaged us, remember it even if it is behind us.
	if (bIsDamageStimulus)
	{
		AddOrUpdateMemory(Actor, Type, true);

		DebugMessage(
			FString::Printf(TEXT("Damage threat remembered: %s Type=%d"),
				*Actor->GetName(),
				static_cast<int32>(Type)),
			FColor::Red);

		return;
	}

	if (bWasSeen)
	{
		AddOrUpdateMemory(Actor, Type, true);

		DebugMessage(
			FString::Printf(TEXT("Saw: %s Type=%d"),
				*Actor->GetName(),
				static_cast<int32>(Type)),
			FColor::Green);
	}
	else
	{
		MarkActorNotVisible(Actor);

		DebugMessage(
			FString::Printf(TEXT("Lost sight of: %s"), *Actor->GetName()),
			FColor::Yellow);
	}
}

void UStudentPerceptorKalraKaran::AddOrUpdateMemory(AActor* Actor, EStudentMemoryType Type, bool bVisible)
{
	if (!Actor)
		return;

	const int32 ExistingIndex = FindMemoryIndex(Actor);
	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	if (ExistingIndex != INDEX_NONE)
	{
		FStudentMemoryEntry& Entry = Memory[ExistingIndex];
		Entry.Actor = Actor;
		Entry.LastKnownLocation = Actor->GetActorLocation();
		Entry.Type = Type;
		Entry.LastSeenTime = CurrentTime;
		Entry.bCurrentlyVisible = bVisible;
		return;
	}

	FStudentMemoryEntry NewEntry;
	NewEntry.Actor = Actor;
	NewEntry.LastKnownLocation = Actor->GetActorLocation();
	NewEntry.Type = Type;
	NewEntry.LastSeenTime = CurrentTime;
	NewEntry.bCurrentlyVisible = bVisible;

	Memory.Add(NewEntry);
}

void UStudentPerceptorKalraKaran::MarkActorNotVisible(AActor* Actor)
{
	if (!Actor)
		return;

	const int32 ExistingIndex = FindMemoryIndex(Actor);

	if (ExistingIndex != INDEX_NONE)
	{
		FStudentMemoryEntry& Entry = Memory[ExistingIndex];
		Entry.LastKnownLocation = Actor->GetActorLocation();
		Entry.LastSeenTime = GetWorld() ? GetWorld()->GetTimeSeconds() : Entry.LastSeenTime;
		Entry.bCurrentlyVisible = false;
	}
}

void UStudentPerceptorKalraKaran::RemoveInvalidMemory()
{
	Memory.RemoveAll(
		[](const FStudentMemoryEntry& Entry)
		{
			return !Entry.Actor.IsValid();
		});
}

int32 UStudentPerceptorKalraKaran::FindMemoryIndex(AActor* Actor) const
{
	if (!Actor)
		return INDEX_NONE;

	for (int32 i = 0; i < Memory.Num(); ++i)
	{
		if (Memory[i].Actor.Get() == Actor)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

EStudentMemoryType UStudentPerceptorKalraKaran::ClassifyActor(AActor* Actor) const
{
	if (!Actor)
		return EStudentMemoryType::Unknown;

	if (IsZombie(Actor))
		return EStudentMemoryType::Zombie;

	if (ABaseItem* Item = Cast<ABaseItem>(Actor))
	{
		switch (Item->GetItemType())
		{
		case EItemType::Pistol:
		case EItemType::Shotgun:
			return EStudentMemoryType::Weapon;

		case EItemType::Medkit:
			return EStudentMemoryType::Medkit;

		case EItemType::Food:
			return EStudentMemoryType::Food;

		case EItemType::Garbage:
			return EStudentMemoryType::Garbage;

		default:
			return EStudentMemoryType::Unknown;
		}
	}

	if (IsHouse(Actor))
		return EStudentMemoryType::House;

	return EStudentMemoryType::Unknown;
}

bool UStudentPerceptorKalraKaran::IsZombie(AActor* Actor) const
{
	return Actor && Actor->IsA(ABaseZombie::StaticClass());
}

bool UStudentPerceptorKalraKaran::IsItem(AActor* Actor) const
{
	return Actor && Actor->IsA(ABaseItem::StaticClass());
}

bool UStudentPerceptorKalraKaran::IsHouse(AActor* Actor) const
{
	if (!Actor)
		return false;

	if (Actor->ActorHasTag(TEXT("House")) || Actor->ActorHasTag(TEXT("Building")))
		return true;

	const FString Name = Actor->GetName();

	return Name.Contains(TEXT("House")) ||
		Name.Contains(TEXT("Building")) ||
		Name.Contains(TEXT("Village"));
}

AActor* UStudentPerceptorKalraKaran::GetBestUnvisitedHouse(const FVector& FromLocation) const
{
	AActor* BestHouse = nullptr;
	float BestScore = -FLT_MAX;

	for (const FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Type != EStudentMemoryType::House)
			continue;

		if (Entry.bVisited)
			continue;

		if (IsMemoryAvoided(Entry))
			continue;

		AActor* House = Entry.Actor.Get();
		if (!House)
			continue;

		const float Score = DistanceScore(FromLocation, Entry.LastKnownLocation);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestHouse = House;
		}
	}

	return BestHouse;
}

void UStudentPerceptorKalraKaran::MarkHouseVisited(AActor* HouseActor)
{
	if (!HouseActor)
		return;

	for (FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Actor.Get() == HouseActor &&
			Entry.Type == EStudentMemoryType::House)
		{
			Entry.bVisited = true;
			Entry.AvoidUntilTime = (GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f) + 300.f;
			return;
		}
	}
}

void UStudentPerceptorKalraKaran::MarkHouseVisitedNearLocation(const FVector& Location, float Radius)
{
	const float RadiusSq = FMath::Square(Radius);

	for (FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Type != EStudentMemoryType::House)
			continue;

		if (Entry.bVisited)
			continue;

		if (FVector::DistSquared(Location, Entry.LastKnownLocation) <= RadiusSq)
		{
			Entry.bVisited = true;

			DebugMessage(
				FString::Printf(TEXT("Marked house visited at %.0f %.0f"),
					Entry.LastKnownLocation.X,
					Entry.LastKnownLocation.Y),
				FColor::Cyan
			);
		}
	}
}

void UStudentPerceptorKalraKaran::AvoidActorForTime(AActor* Actor, float Duration)
{
	if (!Actor)
		return;

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	for (FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Actor.Get() == Actor)
		{
			Entry.AvoidUntilTime = Now + Duration;
			return;
		}
	}
}

bool UStudentPerceptorKalraKaran::IsMemoryAvoided(const FStudentMemoryEntry& Entry) const
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	return Now < Entry.AvoidUntilTime;
}

AActor* UStudentPerceptorKalraKaran::GetNearestKnownZombie(const FVector& FromLocation, bool bOnlyVisible) const
{
	AActor* BestZombie = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();

	for (const FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Type != EStudentMemoryType::Zombie)
			continue;

		if (bOnlyVisible && !Entry.bCurrentlyVisible)
			continue;

		AActor* Zombie = Entry.Actor.Get();
		if (!Zombie)
			continue;

		const float DistSq = FVector::DistSquared(FromLocation, Entry.LastKnownLocation);

		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestZombie = Zombie;
		}
	}

	return BestZombie;
}

void UStudentPerceptorKalraKaran::IgnoreActorForTime(AActor* Actor, float Duration)
{
	if (!Actor)
		return;

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	for (FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Actor.Get() == Actor)
		{
			Entry.AvoidUntilTime = Now + Duration;
			return;
		}
	}
}

AActor* UStudentPerceptorKalraKaran::GetBestKnownItem(
	const FVector& FromLocation,
	bool bNeedsHealing,
	bool bNeedsStamina,
	bool bHasWeapon) const
{

	

	AActor* BestItem = nullptr;
	float BestScore = -FLT_MAX;

	for (const FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Type != EStudentMemoryType::Weapon &&
			Entry.Type != EStudentMemoryType::Medkit &&
			Entry.Type != EStudentMemoryType::Food
			)
		{
			continue;
		}

		if (IsMemoryAvoided(Entry))
			continue;

		AActor* ItemActor = Entry.Actor.Get();
		if (!ItemActor)
			continue;

		const float Score = ScoreItem(ItemActor, FromLocation, bNeedsHealing, bNeedsStamina, bHasWeapon);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestItem = ItemActor;
		}
	}

	return BestItem;
}


AActor* UStudentPerceptorKalraKaran::GetBestKnownGarbage(const FVector& FromLocation) const
{
	AActor* BestGarbage = nullptr;
	float BestScore = -FLT_MAX;

	for (const FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Type != EStudentMemoryType::Garbage)
			continue;

		if (IsMemoryAvoided(Entry))
			continue;

		AActor* GarbageActor = Entry.Actor.Get();
		if (!GarbageActor)
			continue;

		const float Score = DistanceScore(FromLocation, Entry.LastKnownLocation);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestGarbage = GarbageActor;
		}
	}

	return BestGarbage;
}

AActor* UStudentPerceptorKalraKaran::GetBestKnownWeapon(const FVector& FromLocation) const
{
	AActor* BestWeapon = nullptr;
	float BestScore = -FLT_MAX;

	for (const FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Type != EStudentMemoryType::Weapon)
			continue;

		AActor* Actor = Entry.Actor.Get();
		if (!Actor)
			continue;

		const float Score = DistanceScore(FromLocation, Entry.LastKnownLocation);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestWeapon = Actor;
		}
	}

	return BestWeapon;
}

AActor* UStudentPerceptorKalraKaran::GetBestKnownMedkit(const FVector& FromLocation) const
{
	AActor* BestMedkit = nullptr;
	float BestScore = -FLT_MAX;

	for (const FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Type != EStudentMemoryType::Medkit)
			continue;

		AActor* Actor = Entry.Actor.Get();
		if (!Actor)
			continue;

		const float Score = DistanceScore(FromLocation, Entry.LastKnownLocation);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestMedkit = Actor;
		}
	}

	return BestMedkit;
}

AActor* UStudentPerceptorKalraKaran::GetBestKnownFood(const FVector& FromLocation) const
{
	AActor* BestFood = nullptr;
	float BestScore = -FLT_MAX;

	for (const FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Type != EStudentMemoryType::Food)
			continue;

		AActor* Actor = Entry.Actor.Get();
		if (!Actor)
			continue;

		const float Score = DistanceScore(FromLocation, Entry.LastKnownLocation);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestFood = Actor;
		}
	}

	return BestFood;
}

AActor* UStudentPerceptorKalraKaran::GetBestKnownHouse(const FVector& FromLocation) const
{
	AActor* BestHouse = nullptr;
	float BestScore = -FLT_MAX;

	for (const FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Type != EStudentMemoryType::House)
			continue;

		AActor* Actor = Entry.Actor.Get();
		if (!Actor)
			continue;

		const float Score = DistanceScore(FromLocation, Entry.LastKnownLocation);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestHouse = Actor;
		}
	}

	return BestHouse;
}

bool UStudentPerceptorKalraKaran::GetBestKnownLocationOfType(
	EStudentMemoryType Type,
	const FVector& FromLocation,
	FVector& OutLocation) const
{
	bool bFound = false;
	float BestScore = -FLT_MAX;

	for (const FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Type != Type)
			continue;

		const float Score = DistanceScore(FromLocation, Entry.LastKnownLocation);

		if (Score > BestScore)
		{
			BestScore = Score;
			OutLocation = Entry.LastKnownLocation;
			bFound = true;
		}
	}

	return bFound;
}

int32 UStudentPerceptorKalraKaran::GetKnownZombieCount(float MaxDistance, const FVector& FromLocation) const
{
	int32 Count = 0;
	const float MaxDistSq = FMath::Square(MaxDistance);

	for (const FStudentMemoryEntry& Entry : Memory)
	{
		if (Entry.Type != EStudentMemoryType::Zombie)
			continue;

		if (FVector::DistSquared(FromLocation, Entry.LastKnownLocation) <= MaxDistSq)
		{
			++Count;
		}
	}

	return Count;
}

float UStudentPerceptorKalraKaran::ScoreItem(
	AActor* ItemActor,
	const FVector& FromLocation,
	bool bNeedsHealing,
	bool bNeedsStamina,
	bool bHasWeapon) const
{
	if (!ItemActor)
		return -FLT_MAX;

	const int32 Index = FindMemoryIndex(ItemActor);
	if (Index == INDEX_NONE)
		return -FLT_MAX;

	const FStudentMemoryEntry& Entry = Memory[Index];

	float Score = DistanceScore(FromLocation, Entry.LastKnownLocation);

	switch (Entry.Type)
	{
	case EStudentMemoryType::Weapon:
		Score += bHasWeapon ? 150.f : 1000.f;
		break;

	case EStudentMemoryType::Medkit:
		Score += bNeedsHealing ? 900.f : 250.f;
		break;

	case EStudentMemoryType::Food:
		Score += bNeedsStamina ? 700.f : 150.f;
		break;

	case EStudentMemoryType::Garbage:
		Score -= 9999.f;
		break;

	default:
		break;
	}

	return Score;
}

float UStudentPerceptorKalraKaran::DistanceScore(const FVector& FromLocation, const FVector& TargetLocation) const
{
	const float Distance = FVector::Dist(FromLocation, TargetLocation);
	return 1000.f / FMath::Max(Distance, 1.f);
}

void UStudentPerceptorKalraKaran::DebugMessage(const FString& Message, const FColor& Color) const
{
	if (!bDebugMessages || !GEngine)
		return;

	GEngine->AddOnScreenDebugMessage(
		-1,
		1.25f,
		Color,
		Message);
}