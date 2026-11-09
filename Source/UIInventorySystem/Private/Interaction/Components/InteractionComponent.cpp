// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Components/InteractionComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Pickups/ItemPickup.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"

// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	// ...
}


// Called when the game starts
void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFocus();
	// ...
}

AItemPickup* UInteractionComponent::FindFocusedItem(
	TArray<AItemPickup*>* OutCandidates /*= nullptr*/) const
{
	if (OutCandidates)
	{
		OutCandidates->Reset();
	}

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
    {
        return nullptr;
    }

    APlayerController* PlayerController =
        Cast<APlayerController>(OwnerPawn->GetController());

    UWorld* World = GetWorld();
    if (!PlayerController || !World)
    {
        return nullptr;
    }

    int32 ViewportWidth = 0;
    int32 ViewportHeight = 0;
    PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);

    if (ViewportWidth <= 0 || ViewportHeight <= 0)
    {
        return nullptr;
    }

    const FVector2D AimScreenPosition(
        ViewportWidth * 0.5f,
        ViewportHeight * 0.35f
    );

    FVector TraceStart;
    FVector TraceDirection;

    if (!PlayerController->DeprojectScreenPositionToWorld(
        AimScreenPosition.X,
        AimScreenPosition.Y,
        TraceStart,
        TraceDirection))
    {
        return nullptr;
    }

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(OwnerPawn);

    FVector EyesLocation;
    FRotator EyesRotation;
    OwnerPawn->GetActorEyesViewPoint(EyesLocation, EyesRotation);

    // Shared reach check for direct and assisted targets.
    const auto CanReachItem =
        [&](AItemPickup* Item, const FVector& HitPoint) -> bool
        {
            if (!IsValid(Item) || Item->IsActorBeingDestroyed())
            {
                return false;
            }

            if (FVector::DistSquared(
                OwnerPawn->GetActorLocation(), HitPoint)
            > FMath::Square(InteractionDistance))
            {
                return false;
            }

            FCollisionQueryParams ReachParams = QueryParams;
            ReachParams.AddIgnoredActor(Item);

            return !World->LineTraceTestByChannel(
                EyesLocation,
                HitPoint,
                ECC_Visibility,
                ReachParams
            );
        };

    // keep direct-hit priority, but gather other choices when requested.
	AItemPickup* DirectItem = nullptr;
	FHitResult DirectHit;

    if (World->LineTraceSingleByChannel(
        DirectHit,
        TraceStart,
        TraceStart + TraceDirection * TraceDistance,
        ECC_Visibility,
        QueryParams))
    {
		AItemPickup* HitItem = Cast<AItemPickup>(DirectHit.GetActor());
		if (CanReachItem(HitItem, DirectHit.ImpactPoint))
		{
			DirectItem = HitItem;

			if (!OutCandidates)
			{
				return DirectItem; // Return early if we only want the direct hit
			}
			OutCandidates->AddUnique(DirectItem);
		}
    }

    // Otherwise, search near the crosshair.
    const float ScreenRadius =
        ViewportHeight * FocusRadiusScreenFraction;

    const double RadiusSquared = FMath::Square(ScreenRadius);

    AItemPickup* BestItem = nullptr;
    double BestScore = TNumericLimits<double>::Max();

    for (AItemPickup* Item : GatherNearbyItems())
    {
        if (!IsValid(Item) || Item->IsActorBeingDestroyed())
        {
            continue;
        }

        const FVector FocusPoint = Item->GetFocusPoint();
        FVector2D ItemScreenPosition;

        if (!PlayerController->ProjectWorldLocationToScreen(
            FocusPoint, ItemScreenPosition, false))
        {
            continue;
        }

        // Reject points outside the viewport.
        if (ItemScreenPosition.X < 0 ||
            ItemScreenPosition.X > ViewportWidth ||
            ItemScreenPosition.Y < 0 ||
            ItemScreenPosition.Y > ViewportHeight)
        {
            continue;
        }

        const double ScreenDistanceSquared =
            (ItemScreenPosition - AimScreenPosition).SizeSquared();

        if (ScreenDistanceSquared > RadiusSquared)
        {
            continue;
        }

        // Prefer keeping the current item when scores are similar.
        const double Score = ScreenDistanceSquared *
            (Item == FocusedItem.Get() ? 0.8 : 1.0);

        if (!OutCandidates && Score >= BestScore)
        {
            continue;
        }

        // The camera must actually see this item's collision mesh.
        const FVector CandidateDirection =
            (FocusPoint - TraceStart).GetSafeNormal();

        FHitResult CandidateHit;

        if (!World->LineTraceSingleByChannel(
            CandidateHit,
            TraceStart,
            TraceStart + CandidateDirection * TraceDistance,
            ECC_Visibility,
            QueryParams))
        {
            continue;
        }

        if (CandidateHit.GetActor() != Item)
        {
            continue;
        }

        if (!CanReachItem(Item, CandidateHit.ImpactPoint))
        {
            continue;
        }

		if (OutCandidates)
		{
			OutCandidates->AddUnique(Item);
		}

		if (Score < BestScore)
		{
			BestScore = Score;
			BestItem = Item;
		}
    }

    return DirectItem ? DirectItem : BestItem;
}

TArray<AItemPickup*> UInteractionComponent::GatherNearbyItems() const
{
	TArray<AItemPickup*> NearbyItems;

	const AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();

	// If the owner or world is invalid, return an empty array
	if (!IsValid(OwnerActor) || !World) return NearbyItems;

	FCollisionQueryParams QueryParams; // Set up collision query parameters
	QueryParams.AddIgnoredActor(OwnerActor); // Ignore the owner actor in the overlap query

	TArray<FOverlapResult> OverlapResults;// Perform a sphere overlap to find nearby actors

	// Perform a sphere overlap to find nearby actors
	World->OverlapMultiByObjectType(
		OverlapResults,
		OwnerActor->GetActorLocation(),
		FQuat::Identity, // Use identity rotation for the sphere
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects), // Check all object types
		FCollisionShape::MakeSphere(InteractionDistance), // Use a sphere shape for the overlap
		QueryParams // Use the collision query parameters defined above
	);

	// Iterate through the overlap results and filter for AItemPickup actors
	for (const FOverlapResult& Result : OverlapResults)
	{
		AItemPickup* ItemPickup = Cast<AItemPickup>(Result.GetActor());
		if (!IsValid(ItemPickup) || ItemPickup->IsActorBeingDestroyed())
		{
			continue; // Skip invalid or destroyed actors
		}
		NearbyItems.AddUnique(ItemPickup); // Add the valid AItemPickup actor to the NearbyItems array, ensuring uniqueness
	}

	return NearbyItems;
}

void UInteractionComponent::UpdateFocus()
{
	if (ManuallySelectedItem.IsValid())
	{
		TArray<AItemPickup*> Candidates;
		AItemPickup* AutomaticItem = FindFocusedItem(&Candidates);

		AItemPickup* ManualItem = ManuallySelectedItem.Get();

		if (Candidates.Contains(ManualItem))
		{
			SetFocusedItem(ManualItem);
			return;
		}

        // The manual choice is no longer elegible.
		ManuallySelectedItem.Reset();
		SetFocusedItem(AutomaticItem);
        return;
	}
	ManuallySelectedItem.Reset();
	SetFocusedItem(FindFocusedItem());
}

void UInteractionComponent::CycleFocusedItem(int32 Direction)
{
	if (Direction == 0) return;

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	APlayerController* PlayerController =
		Cast<APlayerController>(OwnerPawn->GetController());

    if (!PlayerController) return;

	TArray<AItemPickup*> Candidates;
	AItemPickup* AutomaticItem = FindFocusedItem(&Candidates);

    if (Candidates.Num() < 2)
    {
		// There is nothing to cycle between, so just use the automatic choice.
		ManuallySelectedItem.Reset();
		SetFocusedItem(AutomaticItem);
		return;
    }

	// Order candidates from left to right on the screen.
    Candidates.Sort(
        [PlayerController](const AItemPickup& A, const AItemPickup& B)
        {
			FVector2D ScreenA, ScreenB;

			const bool bProjectedA = PlayerController->ProjectWorldLocationToScreen(
				A.GetFocusPoint(), ScreenA, false);

			const bool bProjectedB = PlayerController->ProjectWorldLocationToScreen(
				B.GetFocusPoint(), ScreenB, false);

			if (bProjectedA != bProjectedB)
			{
				return bProjectedA; // Projected items come first
			}

			if (bProjectedA && bProjectedB)
			{
				if (ScreenA.X != ScreenB.X)
				{
					return ScreenA.X < ScreenB.X; // Sort by X position
				}

				if (ScreenA.Y != ScreenB.Y)
				{
					return ScreenA.Y < ScreenB.Y; // Sort by Y position
				}
			}
			return A.GetUniqueID() < B.GetUniqueID();
        });
	AItemPickup* CurrentItem = ManuallySelectedItem.Get();

    if (!Candidates.Contains(CurrentItem))
    {
		CurrentItem = FocusedItem.Get();
    }
    if (!Candidates.Contains(CurrentItem))
    {
		CurrentItem = AutomaticItem;
    }
	const int32 CurrentIndex = Candidates.IndexOfByKey(CurrentItem);
	const int32 Step = (Direction > 0) ? 1 : -1;

    int32 NextIndex;

    if (CurrentIndex == INDEX_NONE)
    {
		NextIndex = (Step > 0) ? 0 : Candidates.Num() - 1;
	}
	else
	{
		NextIndex = (CurrentIndex + Step + Candidates.Num()) % Candidates.Num();
    }

	ManuallySelectedItem = Candidates[NextIndex];
	SetFocusedItem(Candidates[NextIndex]);
}

void UInteractionComponent::SetFocusedItem(AItemPickup* NewItem)
{
	const TWeakObjectPtr<AItemPickup> NewTarget(NewItem);
	if (FocusedItem.HasSameIndexAndSerialNumber(NewTarget)) return;

	if (AItemPickup* PreviousItem = FocusedItem.Get())
	{
		PreviousItem->SetHighlighted(false);
	}

	FocusedItem = NewTarget;
	if (AItemPickup* CurrentItem = FocusedItem.Get())
	{
		CurrentItem->SetHighlighted(true);
	}
	OnFocusedItemChanged.Broadcast(NewItem); // Notify listeners that the focused item has changed
}

AItemPickup* UInteractionComponent::GetFocusedItem() const
{
	return FocusedItem.Get();
}

void UInteractionComponent::Interact()
{
	// Check again at the exact moment the player requests interaction.
	UpdateFocus();

	AItemPickup* ItemPickup = GetFocusedItem();
	if (ItemPickup && ItemPickup->TryInteract(GetOwner()))
	{
		ManuallySelectedItem.Reset(); // Clear the manual selection if interaction was successful
		SetFocusedItem(nullptr); // Clear the focused item if interaction was successful
	}
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ManuallySelectedItem.Reset(); // Clear the manual selection when the component is destroyed or the game ends
	SetFocusedItem(nullptr); // Clear the focused item when the component is destroyed or the game ends

	Super::EndPlay(EndPlayReason);
}