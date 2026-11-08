// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InteractionPromptWidget.h"
#include "Components/Border.h"
#include "GameFramework/Pawn.h"
#include "Interaction/Components/InteractionComponent.h"
#include "Pickups/ItemPickup.h"
#include "UIInventorySystem.h"

void  UInteractionPromptWidget::HandleFocusedItemChanged(AItemPickup* NewFocusedItem)
{
	if (!PromptContainer) return;

	const bool bCanInteract = IsValid(NewFocusedItem) && !NewFocusedItem->IsActorBeingDestroyed();

	// Update the visibility of the prompt container based on whether the player can interact with the focused item
	PromptContainer->SetVisibility(bCanInteract
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed);
}

void UInteractionPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Remove any previous subscriptions to avoid duplicate bindings
	if (IsValid(BoundInteractionComponent))
	{
		BoundInteractionComponent->OnFocusedItemChanged.RemoveDynamic(this, &UInteractionPromptWidget::HandleFocusedItemChanged);
	}
	BoundInteractionComponent = nullptr;

	HandleFocusedItemChanged(nullptr); // Hide the prompt initially

	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (!IsValid(PlayerPawn)) return;

	BoundInteractionComponent = PlayerPawn->FindComponentByClass<UInteractionComponent>();

	if (!IsValid(BoundInteractionComponent))
	{
		UE_LOG(LogUIInventorySystem, Warning, TEXT("InteractionPromptWidget: No InteractionComponent found on the player pawn."));
		return;
	}

	BoundInteractionComponent->OnFocusedItemChanged.AddUniqueDynamic(
		this, &UInteractionPromptWidget::HandleFocusedItemChanged);

	// The player might already be looking at an interactable item when the widget is constructed, 
	// so we should check the current focused item and update the prompt accordingly.
	HandleFocusedItemChanged(BoundInteractionComponent->GetFocusedItem());

}

void UInteractionPromptWidget::NativeDestruct()
{
	if (IsValid(BoundInteractionComponent))
	{
		BoundInteractionComponent->OnFocusedItemChanged.RemoveDynamic(this, &UInteractionPromptWidget::HandleFocusedItemChanged);
	}
	BoundInteractionComponent = nullptr;
	Super::NativeDestruct();
}
