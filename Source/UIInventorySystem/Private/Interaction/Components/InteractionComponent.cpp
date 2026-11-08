// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Components/InteractionComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Pickups/ItemPickup.h"

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

AItemPickup* UInteractionComponent::FindFocusedItem() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return nullptr;

	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());

	UWorld* World = GetWorld();

	if (!PlayerController || !World) return nullptr;

	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector TraceEnd = CameraLocation + CameraRotation.Vector() * TraceDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	FHitResult HitResult;

	if (!World->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, QueryParams))
	{
		return nullptr;
	}

	AItemPickup* ItemPickup = Cast<AItemPickup>(HitResult.GetActor());

	if(!IsValid(ItemPickup) || ItemPickup->IsActorBeingDestroyed()) return nullptr;

	const float DistanceSquared = FVector::DistSquared(
		OwnerPawn->GetActorLocation(), 
		ItemPickup->GetActorLocation());
	if (DistanceSquared > FMath::Square(InteractionDistance)) return nullptr;

	// Also check that the character can reach it without a wall between them
	FVector EyesLocation;
	FRotator EyesRotation;
	OwnerPawn->GetActorEyesViewPoint(EyesLocation, EyesRotation);

	QueryParams.AddIgnoredActor(ItemPickup);

	if (World->LineTraceTestByChannel(
		EyesLocation,
		HitResult.ImpactPoint,
		ECC_Visibility,
		QueryParams
	)) 
	{
		return nullptr;
	}
	return ItemPickup;
}

void UInteractionComponent::UpdateFocus()
{
	SetFocusedItem(FindFocusedItem());
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
		SetFocusedItem(nullptr); // Clear the focused item if interaction was successful
	}
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetFocusedItem(nullptr); // Clear the focused item when the component is destroyed or the game ends
	
	Super::EndPlay(EndPlayReason);
}