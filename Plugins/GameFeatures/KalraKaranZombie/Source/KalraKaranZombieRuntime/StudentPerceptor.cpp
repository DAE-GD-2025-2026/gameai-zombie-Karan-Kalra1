#include "StudentPerceptor.h"

#include "Engine/Engine.h"
#include "Zombies/BaseZombie.h"
#include "Items/BaseItem.h"

UStudentPerceptor::UStudentPerceptor()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStudentPerceptor::BeginPlay()
{
	Super::BeginPlay();

	PerceptionComponent = GetOwner()->GetComponentByClass<UAIPerceptionComponent>();

	if (PerceptionComponent)
	{
		PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(
			this,
			&UStudentPerceptor::OnPerceptionUpdated
		);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				3.f,
				FColor::Green,
				TEXT("StudentPerceptor connected to AIPerception")
			);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.f,
				FColor::Red,
				TEXT("StudentPerceptor: No AIPerceptionComponent found on owner")
			);
		}
	}
}

void UStudentPerceptor::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor)
		return;

	RemoveInvalidEntries();

	if (Stimulus.WasSuccessfullySensed())
	{
		AddKnownActor(Actor);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				1.f,
				FColor::Green,
				FString::Printf(TEXT("Perceived: %s"), *Actor->GetName())
			);
		}
	}
	else
	{
		RemoveKnownActor(Actor);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				1.f,
				FColor::Yellow,
				FString::Printf(TEXT("Lost perception: %s"), *Actor->GetName())
			);
		}
	}
}

void UStudentPerceptor::AddKnownActor(AActor* Actor)
{
	if (!Actor)
		return;

	if (IsZombie(Actor))
	{
		KnownZombies.AddUnique(Actor);
	}
	else if (IsItem(Actor))
	{
		KnownItems.AddUnique(Actor);
	}
}

void UStudentPerceptor::RemoveKnownActor(AActor* Actor)
{
	if (!Actor)
		return;

	KnownZombies.RemoveAll(
		[Actor](const TWeakObjectPtr<AActor>& Entry)
		{
			return !Entry.IsValid() || Entry.Get() == Actor;
		}
	);

	KnownItems.RemoveAll(
		[Actor](const TWeakObjectPtr<AActor>& Entry)
		{
			return !Entry.IsValid() || Entry.Get() == Actor;
		}
	);
}

void UStudentPerceptor::RemoveInvalidEntries()
{
	KnownZombies.RemoveAll(
		[](const TWeakObjectPtr<AActor>& Entry)
		{
			return !Entry.IsValid();
		}
	);

	KnownItems.RemoveAll(
		[](const TWeakObjectPtr<AActor>& Entry)
		{
			return !Entry.IsValid();
		}
	);
}

bool UStudentPerceptor::IsZombie(AActor* Actor) const
{
	return Actor && Actor->IsA(ABaseZombie::StaticClass());
}

bool UStudentPerceptor::IsItem(AActor* Actor) const
{
	return Actor && Actor->IsA(ABaseItem::StaticClass());
}

AActor* UStudentPerceptor::GetNearestKnownZombie(const FVector& FromLocation) const
{
	AActor* BestZombie = nullptr;
	float BestDistanceSq = TNumericLimits<float>::Max();

	for (const TWeakObjectPtr<AActor>& Entry : KnownZombies)
	{
		AActor* Zombie = Entry.Get();
		if (!Zombie)
			continue;

		const float DistSq = FVector::DistSquared(FromLocation, Zombie->GetActorLocation());

		if (DistSq < BestDistanceSq)
		{
			BestDistanceSq = DistSq;
			BestZombie = Zombie;
		}
	}

	return BestZombie;
}

AActor* UStudentPerceptor::GetBestKnownItem(const FVector& FromLocation) const
{
	AActor* BestItem = nullptr;
	float BestScore = -FLT_MAX;

	for (const TWeakObjectPtr<AActor>& Entry : KnownItems)
	{
		AActor* ItemActor = Entry.Get();
		if (!ItemActor)
			continue;

		const float Distance = FVector::Dist(FromLocation, ItemActor->GetActorLocation());

		float Score = 1000.f / FMath::Max(Distance, 1.f);

		const FString Name = ItemActor->GetName();

		if (Name.Contains(TEXT("Pistol")) || Name.Contains(TEXT("Shotgun")))
		{
			Score += 100.f;
		}
		else if (Name.Contains(TEXT("Medkit")))
		{
			Score += 80.f;
		}
		else if (Name.Contains(TEXT("Food")))
		{
			Score += 40.f;
		}
		else if (Name.Contains(TEXT("Garbage")))
		{
			Score -= 1000.f;
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestItem = ItemActor;
		}
	}

	return BestItem;
}