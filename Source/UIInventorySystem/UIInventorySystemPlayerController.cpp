// Copyright Epic Games, Inc. All Rights Reserved.


#include "UIInventorySystemPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "UIInventorySystem.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AUIInventorySystemPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogUIInventorySystem, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AUIInventorySystemPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AUIInventorySystemPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

bool AUIInventorySystemPlayerController::IsNotALocalPlayerController()
{
	return !IsLocalPlayerController();
}

void AUIInventorySystemPlayerController::ShowInventory()
{
	UE_LOG(LogUIInventorySystem, Log, TEXT("Controller received inventory request"));
	if (!IsLocalPlayerController()) return;
	if (!InventoryWidgetClass)
	{
		UE_LOG(LogUIInventorySystem, Warning, TEXT("InventoryWidgetClass is not assigned"));
		return;
	}
	if (InventoryWidget == nullptr)
	{
		InventoryWidget = CreateWidget<UUserWidget>(this, InventoryWidgetClass);
	}
	if (InventoryWidget && !InventoryWidget->IsInViewport())
	{
		InventoryWidget->AddToPlayerScreen();
		bShowMouseCursor = true;
		SetIgnoreLookInput(true);

		//UI only
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
		SetInputMode(InputMode);
	}
}

void AUIInventorySystemPlayerController::HideInventory()
{
	if (IsNotALocalPlayerController()) return;

	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromParent();
		bShowMouseCursor = false;
		SetIgnoreLookInput(false);
		SetInputMode(FInputModeGameOnly());
	}
}

void AUIInventorySystemPlayerController::ToggleInventory()
{
	if (IsNotALocalPlayerController()) return;

	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		HideInventory();
	}
	else
	{
		ShowInventory();
	}
}
