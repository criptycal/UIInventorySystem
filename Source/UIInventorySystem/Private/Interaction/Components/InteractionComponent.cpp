#include "Interaction/Components/InteractionComponent.h"

#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Pickups/ItemPickup.h"

namespace InteractionFocus
{
	// Keep this position aligned with CrosshairContainer's anchors in WBP_PlayerHUD.
	constexpr float AimScreenX = 0.5f;
	constexpr float AimScreenY = 0.35f;
	constexpr double CurrentTargetScoreMultiplier = 0.8;
}

struct UInteractionComponent::FFocusContext
{
	APawn* OwnerPawn = nullptr;
	APlayerController* PlayerController = nullptr;
	UWorld* World = nullptr;
	FVector OwnerLocation = FVector::ZeroVector;
	FVector OwnerForward = FVector::ForwardVector;
	double MinimumFacingDot = 0.0;
	FVector EyesLocation = FVector::ZeroVector;
	FVector TraceStart = FVector::ZeroVector;
	FVector TraceDirection = FVector::ZeroVector;
	FVector2D ViewportSize = FVector2D::ZeroVector;
	FVector2D AimScreenPosition = FVector2D::ZeroVector;
	double RadiusSquared = 0.0;
	FCollisionQueryParams TraceParams;
};

struct UInteractionComponent::FFocusCandidate
{
	AItemPickup* Item = nullptr;
	FVector FocusPoint = FVector::ZeroVector;
	FVector2D ScreenPosition = FVector2D::ZeroVector;
	double Score = TNumericLimits<double>::Max();
	bool bProjected = false;

	bool operator<(const FFocusCandidate& Other) const
	{
		if (bProjected != Other.bProjected)
		{
			return bProjected;
		}
		if (bProjected)
		{
			if (ScreenPosition.X != Other.ScreenPosition.X)
			{
				return ScreenPosition.X < Other.ScreenPosition.X;
			}
			if (ScreenPosition.Y != Other.ScreenPosition.Y)
			{
				return ScreenPosition.Y < Other.ScreenPosition.Y;
			}
		}
		return Item->GetUniqueID() < Other.Item->GetUniqueID();
	}
};

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFocus();
}

bool UInteractionComponent::BuildFocusContext(FFocusContext& OutContext) const
{
	OutContext.OwnerPawn = Cast<APawn>(GetOwner());
	if (!OutContext.OwnerPawn || !OutContext.OwnerPawn->IsLocallyControlled())
	{
		return false;
	}

	OutContext.PlayerController = Cast<APlayerController>(OutContext.OwnerPawn->GetController());
	OutContext.World = GetWorld();
	if (!OutContext.PlayerController || !OutContext.World)
	{
		return false;
	}

	int32 ViewportWidth = 0;
	int32 ViewportHeight = 0;
	OutContext.PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);
	if (ViewportWidth <= 0 || ViewportHeight <= 0)
	{
		return false;
	}

	OutContext.ViewportSize = FVector2D(ViewportWidth, ViewportHeight);
	OutContext.AimScreenPosition = FVector2D(
		ViewportWidth * InteractionFocus::AimScreenX,
		ViewportHeight * InteractionFocus::AimScreenY);
	if (!OutContext.PlayerController->DeprojectScreenPositionToWorld(
		OutContext.AimScreenPosition.X, OutContext.AimScreenPosition.Y,
		OutContext.TraceStart, OutContext.TraceDirection))
	{
		return false;
	}

	OutContext.OwnerLocation = OutContext.OwnerPawn->GetActorLocation();
	OutContext.OwnerForward = OutContext.OwnerPawn->GetActorForwardVector().GetSafeNormal2D();
	const double HalfAngle = FMath::Clamp(static_cast<double>(InteractionHalfAngleDegrees), 0.0, 90.0);
	OutContext.MinimumFacingDot = FMath::Max(0.0, FMath::Cos(FMath::DegreesToRadians(HalfAngle)));
	FRotator EyesRotation;
	OutContext.OwnerPawn->GetActorEyesViewPoint(OutContext.EyesLocation, EyesRotation);
	OutContext.RadiusSquared = FMath::Square(ViewportHeight * FocusRadiusScreenFraction);
	OutContext.TraceParams.bTraceComplex = true;
	OutContext.TraceParams.AddIgnoredActor(OutContext.OwnerPawn);
	return true;
}

bool UInteractionComponent::CanReachItem(
	const FFocusContext& Context, AItemPickup* Item, const FVector& HitPoint) const
{
	if (!IsValid(Item) || Item->IsActorBeingDestroyed())
	{
		return false;
	}
	if (FVector::DistSquared(Context.OwnerLocation, HitPoint) > FMath::Square(InteractionDistance))
	{
		return false;
	}

	// Use character yaw, not camera aim; height must not exclude floor/table items.
	const FVector DirectionToItem = (Item->GetFocusPoint() - Context.OwnerLocation).GetSafeNormal2D();
	if (!DirectionToItem.IsNearlyZero() &&
		FVector::DotProduct(Context.OwnerForward, DirectionToItem) < Context.MinimumFacingDot)
	{
		return false;
	}

	// The target itself must not obstruct the character-to-hit-point check.
	FCollisionQueryParams ReachParams = Context.TraceParams;
	ReachParams.AddIgnoredActor(Item);
	return !Context.World->LineTraceTestByChannel(
		Context.EyesLocation, HitPoint, ECC_Visibility, ReachParams);
}

AItemPickup* UInteractionComponent::FindDirectTarget(const FFocusContext& Context) const
{
	FHitResult Hit;
	if (!Context.World->LineTraceSingleByChannel(
		Hit, Context.TraceStart, Context.TraceStart + Context.TraceDirection * TraceDistance,
		ECC_Visibility, Context.TraceParams))
	{
		return nullptr;
	}

	AItemPickup* Item = Cast<AItemPickup>(Hit.GetActor());
	return CanReachItem(Context, Item, Hit.ImpactPoint) ? Item : nullptr;
}

TArray<AItemPickup*> UInteractionComponent::GatherNearbyItems(const FFocusContext& Context) const
{
	FCollisionQueryParams OverlapParams;
	OverlapParams.AddIgnoredActor(Context.OwnerPawn);
	TArray<FOverlapResult> Overlaps;
	Context.World->OverlapMultiByObjectType(
		Overlaps, Context.OwnerLocation, FQuat::Identity,
		FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects),
		FCollisionShape::MakeSphere(InteractionDistance), OverlapParams);

	TArray<AItemPickup*> NearbyItems;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AItemPickup* Item = Cast<AItemPickup>(Overlap.GetActor());
		if (IsValid(Item) && !Item->IsActorBeingDestroyed())
		{
			NearbyItems.AddUnique(Item);
		}
	}
	return NearbyItems;
}

UInteractionComponent::FFocusCandidate UInteractionComponent::MakeFocusCandidate(
	const FFocusContext& Context, AItemPickup* Item) const
{
	FFocusCandidate Candidate;
	Candidate.Item = Item;
	Candidate.FocusPoint = Item->GetFocusPoint();
	Candidate.bProjected = Context.PlayerController->ProjectWorldLocationToScreen(
		Candidate.FocusPoint, Candidate.ScreenPosition, false);
	if (Candidate.bProjected)
	{
		const double ScreenDistanceSquared =
			(Candidate.ScreenPosition - Context.AimScreenPosition).SizeSquared();
		// A small preference for the current focus reduces automatic switching.
		Candidate.Score = ScreenDistanceSquared * (Item == FocusedItem.Get()
			? InteractionFocus::CurrentTargetScoreMultiplier : 1.0);
	}
	return Candidate;
}

bool UInteractionComponent::EvaluateCandidate(
	const FFocusContext& Context, AItemPickup* Item, double ScoreLimit, FFocusCandidate& OutCandidate) const
{
	if (!IsValid(Item) || Item->IsActorBeingDestroyed())
	{
		return false;
	}
	OutCandidate = MakeFocusCandidate(Context, Item);
	const FVector2D& ScreenPosition = OutCandidate.ScreenPosition;
	if (!OutCandidate.bProjected ||
		ScreenPosition.X < 0 || ScreenPosition.X > Context.ViewportSize.X ||
		ScreenPosition.Y < 0 || ScreenPosition.Y > Context.ViewportSize.Y)
	{
		return false;
	}
	if ((ScreenPosition - Context.AimScreenPosition).SizeSquared() > Context.RadiusSquared ||
		OutCandidate.Score >= ScoreLimit)
	{
		return false;
	}

	const FVector Direction = (OutCandidate.FocusPoint - Context.TraceStart).GetSafeNormal();
	FHitResult Hit;
	if (!Context.World->LineTraceSingleByChannel(
		Hit, Context.TraceStart, Context.TraceStart + Direction * TraceDistance,
		ECC_Visibility, Context.TraceParams) || Hit.GetActor() != Item)
	{
		return false;
	}
	return CanReachItem(Context, Item, Hit.ImpactPoint);
}

AItemPickup* UInteractionComponent::FindFocusedItem(TArray<FFocusCandidate>* OutCandidates) const
{
	if (OutCandidates)
	{
		OutCandidates->Reset();
	}
	FFocusContext Context;
	if (!BuildFocusContext(Context))
	{
		return nullptr;
	}

	AItemPickup* DirectItem = FindDirectTarget(Context);
	if (DirectItem)
	{
		if (!OutCandidates)
		{
			return DirectItem;
		}
		// Direct hits remain eligible even if their mesh center is outside the assist circle.
		OutCandidates->Add(MakeFocusCandidate(Context, DirectItem));
	}

	AItemPickup* BestItem = nullptr;
	double BestScore = TNumericLimits<double>::Max();
	for (AItemPickup* Item : GatherNearbyItems(Context))
	{
		if (Item == DirectItem)
		{
			continue;
		}
		FFocusCandidate Candidate;
		// Cycling needs all eligible choices; automatic focus can skip worse scores early.
		const double ScoreLimit = OutCandidates ? TNumericLimits<double>::Max() : BestScore;
		if (!EvaluateCandidate(Context, Item, ScoreLimit, Candidate))
		{
			continue;
		}
		if (OutCandidates)
		{
			OutCandidates->Add(Candidate);
		}
		if (Candidate.Score < BestScore)
		{
			BestItem = Item;
			BestScore = Candidate.Score;
		}
	}
	return DirectItem ? DirectItem : BestItem;
}

int32 UInteractionComponent::FindCandidateIndex(const TArray<FFocusCandidate>& Candidates, AItemPickup* Item)
{
	return Candidates.IndexOfByPredicate([Item](const FFocusCandidate& Candidate)
	{
		return Candidate.Item == Item;
	});
}

int32 UInteractionComponent::FindCycleStartIndex(
	const TArray<FFocusCandidate>& Candidates, AItemPickup* AutomaticItem) const
{
	for (AItemPickup* Item : {ManuallySelectedItem.Get(), FocusedItem.Get(), AutomaticItem})
	{
		const int32 Index = FindCandidateIndex(Candidates, Item);
		if (Index != INDEX_NONE)
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

void UInteractionComponent::UpdateFocus()
{
	if (ManuallySelectedItem.IsValid())
	{
		TArray<FFocusCandidate> Candidates;
		AItemPickup* AutomaticItem = FindFocusedItem(&Candidates);
		AItemPickup* ManualItem = ManuallySelectedItem.Get();
		if (FindCandidateIndex(Candidates, ManualItem) != INDEX_NONE)
		{
			SetFocusedItem(ManualItem);
			return;
		}
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
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled() ||
		!Cast<APlayerController>(OwnerPawn->GetController()))
	{
		return;
	}

	// Gather all eligible candidates, including the current automatic choice, and sort them by screen position.
	TArray<FFocusCandidate> Candidates;
	AItemPickup* AutomaticItem = FindFocusedItem(&Candidates);
	if (Candidates.Num() < 2)
	{
		ManuallySelectedItem.Reset();
		SetFocusedItem(AutomaticItem);
		return;
	}

	// Screen positions were captured during evaluation, so comparisons do no projections.
	Candidates.Sort();
	const int32 CurrentIndex = FindCycleStartIndex(Candidates, AutomaticItem);
	const int32 Step = Direction > 0 ? 1 : -1;
	const int32 NextIndex = CurrentIndex == INDEX_NONE
		? (Step > 0 ? 0 : Candidates.Num() - 1)
		: (CurrentIndex + Step + Candidates.Num()) % Candidates.Num();

	ManuallySelectedItem = Candidates[NextIndex].Item;
	SetFocusedItem(Candidates[NextIndex].Item);
}

void UInteractionComponent::SetFocusedItem(AItemPickup* NewItem)
{
	const TWeakObjectPtr<AItemPickup> NewTarget(NewItem);
	if (FocusedItem.HasSameIndexAndSerialNumber(NewTarget))
	{
		return;
	}
	if (AItemPickup* PreviousItem = FocusedItem.Get())
	{
		PreviousItem->SetHighlighted(false);
	}
	FocusedItem = NewTarget;
	if (AItemPickup* CurrentItem = FocusedItem.Get())
	{
		CurrentItem->SetHighlighted(true);
	}
	OnFocusedItemChanged.Broadcast(NewItem);
}

AItemPickup* UInteractionComponent::GetFocusedItem() const
{
	return FocusedItem.Get();
}

void UInteractionComponent::Interact()
{
	// Revalidate the automatic or manual choice at the moment of interaction.
	UpdateFocus();
	AItemPickup* ItemPickup = GetFocusedItem();
	if (ItemPickup && ItemPickup->TryInteract(GetOwner()))
	{
		ManuallySelectedItem.Reset();
		SetFocusedItem(nullptr);
	}
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ManuallySelectedItem.Reset();
	SetFocusedItem(nullptr);
	Super::EndPlay(EndPlayReason);
}
