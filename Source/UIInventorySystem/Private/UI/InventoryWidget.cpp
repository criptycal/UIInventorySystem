// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventoryWidget.h"
#include "UIInventorySystemPlayerController.h"
#include "InputCoreTypes.h"
#include "Input/Events.h"
// #include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "Inventory/Components/CurrencyComponent.h"


void UInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Visible);

	// if (Button_0)
	// {
	// 	Button_0->OnClicked.AddDynamic(this, &UInventoryWidget::HandleCloseButtonClicked);
	// }


}

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	bCloseOnTabRelease = false;

	if (!MoneyText) return;

	int32 BalanceToDisplay = 0;

	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (PlayerPawn)
	{
		UCurrencyComponent* CurrencyComp = PlayerPawn->FindComponentByClass<UCurrencyComponent>();
		if (CurrencyComp)
		{
			BalanceToDisplay = CurrencyComp->GetBalance();
		}
	}
	MoneyText->SetText(FText::AsNumber(BalanceToDisplay));
}

FReply UInventoryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{

	SetKeyboardFocus();
	return FReply::Handled();
}

FReply UInventoryWidget::NativeOnPreviewKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() != EKeys::Tab)
	{
		return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
	}
	
	bool bIsInKeyEventRepeat = InKeyEvent.IsRepeat();

	if (!bIsInKeyEventRepeat)
	{
		bCloseOnTabRelease = true;
	}
	

	return FReply::Handled();
}

FReply UInventoryWidget::NativeOnKeyUp(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() != EKeys::Tab)
	{
		return Super::NativeOnKeyUp(InGeometry, InKeyEvent);
	}

	if (!bCloseOnTabRelease)
	{
		return FReply::Unhandled();
	}

	bCloseOnTabRelease = false;

	AUIInventorySystemPlayerController* PlayerController = Cast<AUIInventorySystemPlayerController>(GetOwningPlayer());

	if (PlayerController)
	{
		PlayerController->HideInventory();
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

// void UInventoryWidget::HandleCloseButtonClicked()
// {
// 	AUIInventorySystemPlayerController* PlayerController = Cast<AUIInventorySystemPlayerController>(GetOwningPlayer());
// 	if (PlayerController)
// 	{
// 		PlayerController->HideInventory();
// 	}
// }

