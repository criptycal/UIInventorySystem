// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventoryWidget.h"
#include "UIInventorySystemPlayerController.h"
#include "InputCoreTypes.h"
#include "Input/Events.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"
#include "Inventory/Components/CurrencyComponent.h"
#include "Inventory/Components/HealthComponent.h"
#include "Components/Image.h"
#include "Components/WrapBox.h"
#include "UI/ItemSlotWidget.h"
#include "Inventory/Components/InventoryComponent.h"
#include "Inventory/Data/ItemRow.h"
#include "UIInventorySystem.h"


void UInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Visible);

	if (DamageTestButton)
	{
		
		DamageTestButton->OnClicked.AddUniqueDynamic(this, &UInventoryWidget::HandleDamageTestButtonClicked);
	}

	if (HealTestButton)
	{
		
		HealTestButton->OnClicked.AddUniqueDynamic(this, &UInventoryWidget::HandleHealTestButtonClicked);
	}

	// if (Button_0)
	// {
	// 	Button_0->OnClicked.AddDynamic(this, &UInventoryWidget::HandleCloseButtonClicked);
	// }

	if (WeaponsFilterButton)
	{
		WeaponsFilterButton->OnClicked.AddUniqueDynamic(this, &UInventoryWidget::HandleWeaponsFilterClicked);
	}

	if (ShieldsFilterButton)
	{
		ShieldsFilterButton->OnClicked.AddUniqueDynamic(this, &UInventoryWidget::HandleShieldsFilterClicked);
	}

	if (FoodFilterButton)
	{
		FoodFilterButton->OnClicked.AddUniqueDynamic(this, &UInventoryWidget::HandleFoodFilterClicked);
	}

	UButton* CategoryButtons[] = { WeaponsFilterButton, ShieldsFilterButton, FoodFilterButton };

	for (UButton* Button : CategoryButtons)
	{
		if (Button)
		{
			Button->OnHovered.AddDynamic(this, &UInventoryWidget::UpdateCategoryVisuals);
			Button->OnUnhovered.AddDynamic(this, &UInventoryWidget::UpdateCategoryVisuals);
		}
	}

	if (PreviousPageButton)
	{
		PreviousPageButton->OnClicked.AddUniqueDynamic(this, &UInventoryWidget::HandlePreviousPageClicked);
	}

	if (NextPageButton)
	{
		NextPageButton->OnClicked.AddUniqueDynamic(this, &UInventoryWidget::HandleNextPageClicked);
	}

	UpdatePageButtons();
}

void UInventoryWidget::HandlePreviousPageClicked()
{
	if (CurrentPageIndex <= 0) return;

	--CurrentPageIndex;
	RebuildItemGrid();
}

void UInventoryWidget::HandleNextPageClicked()
{
	if (TotalPages <= 0 || CurrentPageIndex >= TotalPages - 1) return;

	++CurrentPageIndex;
	RebuildItemGrid();
}

void UInventoryWidget::UpdatePageButtons()
{
	if (PreviousPageButton)
	{
		PreviousPageButton->SetIsEnabled(TotalPages > 0 && CurrentPageIndex > 0);
	}

	if (NextPageButton)
	{
		NextPageButton->SetIsEnabled(TotalPages > 0 && CurrentPageIndex < TotalPages - 1);
	}
}


void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	bCloseOnTabRelease = false;
	bCloseOnBRelease = false;

	SelectCategory(CurrentCategory);

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
	const FKey Key = InKeyEvent.GetKey();

	if (Key != EKeys::Tab && Key != EKeys::B)
	{
		return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
	}

	if (!InKeyEvent.IsRepeat())
	{
		if (Key == EKeys::Tab)
		{
			bCloseOnTabRelease = true;
		}
		else if (Key == EKeys::B)
		{
			bCloseOnBRelease = true;
		}
	}

	return FReply::Handled();
}

FReply UInventoryWidget::NativeOnKeyUp(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key != EKeys::Tab && Key != EKeys::B)
	{
		return Super::NativeOnKeyUp(InGeometry, InKeyEvent);
	}

	const bool bShouldClose =
		(Key == EKeys::Tab && bCloseOnTabRelease) ||
		(Key == EKeys::B && bCloseOnBRelease);

	if (!bShouldClose)
	{
		return FReply::Unhandled();
	}
	bCloseOnTabRelease = false;
	bCloseOnBRelease = false;

	AUIInventorySystemPlayerController* PlayerController = Cast<AUIInventorySystemPlayerController>(GetOwningPlayer());
	if (PlayerController)
	{
		PlayerController->HideInventory();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UInventoryWidget::HandleDamageTestButtonClicked()
{
	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (PlayerPawn)
	{
		UHealthComponent* HealthComp = PlayerPawn->FindComponentByClass<UHealthComponent>();
		if (HealthComp)
		{
			HealthComp->ApplyDamage(0.5f);
		}
	}
}

void UInventoryWidget::HandleHealTestButtonClicked()
{
	APawn* PlayerPawn = GetOwningPlayerPawn();
	if (PlayerPawn)
	{
		UHealthComponent* HealthComp = PlayerPawn->FindComponentByClass<UHealthComponent>();
		if (HealthComp)
		{
			HealthComp->Heal(0.5f);
		}
	}
}

void UInventoryWidget::HandleWeaponsFilterClicked()
{
	SelectCategory(EItemCategory::Weapon);
}

void UInventoryWidget::HandleShieldsFilterClicked()
{
	SelectCategory(EItemCategory::Shields);
}

void UInventoryWidget::HandleFoodFilterClicked()
{
	SelectCategory(EItemCategory::Food);
}

void UInventoryWidget::SelectCategory(EItemCategory NewCategory)
{
	FText NewTitle;

	switch (NewCategory)
	{
	case EItemCategory::Weapon:
		NewTitle = NSLOCTEXT("Inventory", "WeaponsCategory", "Weapons");
		break;
	case EItemCategory::Shields:
		NewTitle = NSLOCTEXT("Inventory", "ShieldsCategory", "Shields");
		break;
	case EItemCategory::Food:
		NewTitle = NSLOCTEXT("Inventory", "FoodCategory", "Food");
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("Invalid inventory category selected."));
		return;
	}

	CurrentCategory = NewCategory;

	CurrentPageIndex = 0;

	if (CategoryTitle)
	{
		CategoryTitle->SetText(NewTitle);
	}

	UpdateCategoryVisuals();
	RebuildItemGrid();
}

void UInventoryWidget::UpdateCategoryVisuals()
{
	const auto UpdateIcon = [this](UImage* Icon, UButton* Button, EItemCategory Category)
		{
			if (!Icon) return;

			float Opacity = 0.4f;

			if (CurrentCategory == Category)
			{
				Opacity = 1.0f;
			}
			else if (Button && Button->IsHovered())
			{
				Opacity = 0.7f;
			}
			Icon->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, Opacity));
		};
	UpdateIcon(WeaponsFilterIcon, WeaponsFilterButton, EItemCategory::Weapon);
	UpdateIcon(ShieldsFilterIcon, ShieldsFilterButton, EItemCategory::Shields);
	UpdateIcon(FoodFilterIcon, FoodFilterButton, EItemCategory::Food);

}

void UInventoryWidget::RebuildItemGrid()
{
	TotalPages = 0;
	UpdatePageButtons();

	if (!ItemGrid) return;

	ItemGrid->ClearChildren();

	if (!ItemSlotClass)
	{
		UE_LOG(LogUIInventorySystem, Warning,
			TEXT("Cannot display inventory: ItemSlotClass is not assigned."));
		return;
	}

	APawn* PlayerPawn = GetOwningPlayerPawn();

	if (!PlayerPawn)
	{
		return;
	}

	UInventoryComponent* Inventory =
		PlayerPawn->FindComponentByClass<UInventoryComponent>();

	if (!Inventory)
	{
		UE_LOG(LogUIInventorySystem, Warning,
			TEXT("Cannot display inventory: owning pawn has no InventoryComponent."));
		return;
	}

	//Gather the occupied entries belonging to the current category
	TArray<FInventoryEntry> CategoryEntries;

	for (const FInventoryEntry& Entry : Inventory->GetEntries())
	{
		if (Entry.Quantity <= 0) continue;

		const FItemRow* Definition =
			Inventory->FindItemDefinition(Entry.ItemRowName);

		if (Definition && Definition->Category == CurrentCategory) CategoryEntries.Add(Entry);
	}

	const int32 EntryCount = CategoryEntries.Num();

	TotalPages = EntryCount > 0 ? 1 + (EntryCount - 1) / ItemsPerPage : 0;

	CurrentPageIndex = FMath::Clamp(CurrentPageIndex, 0, FMath::Max(0, TotalPages - 1));

	UpdatePageButtons();

	//End index is exclusive: stop before reaching it
	const int32 StartIndex = CurrentPageIndex * ItemsPerPage;
	const int32 EndIndex = StartIndex +
		FMath::Min(ItemsPerPage, EntryCount - StartIndex);

	for (int32 Index = StartIndex; Index < EndIndex; ++Index)
	{
		const FInventoryEntry& Entry = CategoryEntries[Index];

		const FItemRow* Definition = Inventory->FindItemDefinition(Entry.ItemRowName);

		if (!Definition)
		{
			UE_LOG(LogUIInventorySystem, Warning,
				TEXT("Cannot display inventory: item definition not found for row name %s."),
				*Entry.ItemRowName.ToString());
			continue;
		}

		UItemSlotWidget* SlotWidget = CreateWidget<UItemSlotWidget>(GetOwningPlayer(), ItemSlotClass);
		if (!SlotWidget)
		{
			UE_LOG(LogUIInventorySystem, Warning,
				TEXT("Cannot display inventory: failed to create item slot widget."));
			continue;
		}

		SlotWidget->SetItemDisplay(Definition->Icon.Get(), Entry.Quantity);

		ItemGrid->AddChildToWrapBox(SlotWidget);
	}
}



// void UInventoryWidget::HandleCloseButtonClicked()
// {
// 	AUIInventorySystemPlayerController* PlayerController = Cast<AUIInventorySystemPlayerController>(GetOwningPlayer());
// 	if (PlayerController)
// 	{
// 		PlayerController->HideInventory();
// 	}
// }

