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
#include "Components/Border.h"
#include "Components/ScrollBox.h"
#include "Components/HorizontalBox.h"
#include "Engine/Texture2D.h"
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

	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		ObservedInventory = Pawn->FindComponentByClass<UInventoryComponent>();
		if (ObservedInventory.IsValid())
		{
			ObservedInventory->OnInventoryChanged.RemoveAll(this);
			ObservedInventory->OnInventoryChanged.AddUObject(this, &UInventoryWidget::HandleInventoryChanged);
		}
	}
	CreateCharacterPreview();
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


void UInventoryWidget::NativeDestruct()
{
	if (ObservedInventory.IsValid()) ObservedInventory->OnInventoryChanged.RemoveAll(this);
	ObservedInventory.Reset();
	ClearItemSelection();
	ReleaseCharacterPreview();
	Super::NativeDestruct();
}

FReply UInventoryWidget::NativeOnPreviewKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Enter || Key == EKeys::A)
	{
		if (!InKeyEvent.IsRepeat()) OpenActionMenu();
		return FReply::Handled();
	}

	if (Key == EKeys::Y)
	{
		if (!InKeyEvent.IsRepeat()) SortCurrentCategory();
		return FReply::Handled();
	}

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

void UInventoryWidget::SortCurrentCategory()
{
	SortedCategories.Add(CurrentCategory);
	CurrentPageIndex = 0;
	RebuildItemGrid(); // Rebuilding also clears selection and the detail panel.
	SetKeyboardFocus();
}

void UInventoryWidget::SortCategoryEntries(
	TArray<FInventoryEntry>& Entries, const UInventoryComponent& Inventory) const
{
	// Sort the complete category before pagination, without modifying inventory storage.
	Entries.StableSort([&Inventory](const FInventoryEntry& Left, const FInventoryEntry& Right)
	{
		const FItemRow* LeftDefinition = Inventory.FindItemDefinition(Left.ItemRowName);
		const FItemRow* RightDefinition = Inventory.FindItemDefinition(Right.ItemRowName);
		// Entries were validated when gathered; keep a consistent order if a definition is missing.
		if (!LeftDefinition || !RightDefinition)
		{
			return LeftDefinition && !RightDefinition;
		}
		if (LeftDefinition->SortPriority != RightDefinition->SortPriority)
		{
			return LeftDefinition->SortPriority < RightDefinition->SortPriority;
		}
		const int32 NameComparison = LeftDefinition->DisplayName.CompareTo(RightDefinition->DisplayName);
		if (NameComparison != 0) return NameComparison < 0;
		// Stable sorting preserves separate copies/stacks of the same item in their original order.
		if (Left.ItemRowName != Right.ItemRowName)
		{
			return Left.ItemRowName.LexicalLess(Right.ItemRowName);
		}
		// Keep each equipment type together, with its stronger rolls first.
		return LeftDefinition->ResolveEquipmentPower(Left.EquipmentPower)
			> RightDefinition->ResolveEquipmentPower(Right.EquipmentPower);
	});
}

void UInventoryWidget::RebuildItemGrid()
{
	ClearItemSelection();
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

	if (SortedCategories.Contains(CurrentCategory)) SortCategoryEntries(CategoryEntries, *Inventory);

	const int32 EntryCount = CategoryEntries.Num();
	// Display available capacity as frames; empty frames are not inventory entries.
	// Keep existing items visible if a designer later lowers capacity below usage.
	const int32 SlotCount = FMath::Max(EntryCount, Inventory->GetSlotCapacity(CurrentCategory));

	TotalPages = SlotCount > 0 ? 1 + (SlotCount - 1) / ItemsPerPage : 0;

	CurrentPageIndex = FMath::Clamp(CurrentPageIndex, 0, FMath::Max(0, TotalPages - 1));

	UpdatePageButtons();

	//End index is exclusive: stop before reaching it
	const int32 StartIndex = CurrentPageIndex * ItemsPerPage;
	const int32 EndIndex = StartIndex +
		FMath::Min(ItemsPerPage, SlotCount - StartIndex);

	for (int32 Index = StartIndex; Index < EndIndex; ++Index)
	{
		UItemSlotWidget* SlotWidget = CreateWidget<UItemSlotWidget>(GetOwningPlayer(), ItemSlotClass);
		if (!SlotWidget)
		{
			UE_LOG(LogUIInventorySystem, Warning,
				TEXT("Cannot display inventory: failed to create item slot widget."));
			continue;
		}

		SlotWidget->SetEmptyDisplay();
		if (CategoryEntries.IsValidIndex(Index))
		{
			const FInventoryEntry& Entry = CategoryEntries[Index];
			if (const FItemRow* Definition = Inventory->FindItemDefinition(Entry.ItemRowName))
			{
				SlotWidget->SetItemDisplay(Entry.ItemRowName, *Definition, Entry.Quantity);
				VisibleEntryIds.Add(SlotWidget, Entry.EntryId);
				SlotWidget->OnItemClicked.BindUObject(this, &UInventoryWidget::HandleItemClicked);
				SlotWidget->OnHoverChanged.BindUObject(this, &UInventoryWidget::HandleItemHoverChanged);
			}
		}

		ItemGrid->AddChildToWrapBox(SlotWidget);
	}
}

void UInventoryWidget::ClearItemSelection()
{
	CloseActionMenu();
	VisibleEntryIds.Reset();
	// Detach callbacks before removing children: removal can produce a late mouse-leave event.
	if (ItemGrid)
	{
		for (UWidget* Child : ItemGrid->GetAllChildren())
		{
			if (UItemSlotWidget* SlotWidget = Cast<UItemSlotWidget>(Child))
			{
				SlotWidget->OnItemClicked.Unbind();
				SlotWidget->OnHoverChanged.Unbind();
				SlotWidget->SetSelected(false);
			}
		}
	}
	SelectedSlot.Reset();
	HoveredSlot.Reset();
	RefreshItemInfo();
}

void UInventoryWidget::HandleItemClicked(UItemSlotWidget* SlotWidget)
{
	if (!IsValid(SlotWidget) || !ItemGrid || ItemGrid->GetChildIndex(SlotWidget) == INDEX_NONE) return;
	if (UItemSlotWidget* Previous = SelectedSlot.Get())
	{
		Previous->SetSelected(false);
	}
	SelectedSlot = SlotWidget;
	SlotWidget->SetSelected(true);
	SetKeyboardFocus(); // Keep Tab/B available after interacting with inventory content.
	RefreshItemInfo();
	OpenActionMenu();
}

void UInventoryWidget::HandleItemHoverChanged(UItemSlotWidget* SlotWidget, bool bHovered)
{
	if (ActionMenu) return;
	if (!IsValid(SlotWidget) || !ItemGrid || ItemGrid->GetChildIndex(SlotWidget) == INDEX_NONE) return;
	if (bHovered)
	{
		HoveredSlot = SlotWidget;
	}
	else if (HoveredSlot.Get() == SlotWidget)
	{
		HoveredSlot.Reset();
	}
	RefreshItemInfo();
}

void UInventoryWidget::RefreshItemInfo()
{
	if (!ItemInfoPanel || !ItemNameText || !ItemQuantityText || !ItemDescriptionText
		|| !ItemStatRow || !ItemStatValueText) return;

	UItemSlotWidget* Preview = HoveredSlot.IsValid() ? HoveredSlot.Get() : SelectedSlot.Get();
	const APawn* PlayerPawn = GetOwningPlayerPawn();
	const UInventoryComponent* Inventory = PlayerPawn ? PlayerPawn->FindComponentByClass<UInventoryComponent>() : nullptr;
	const FGuid* EntryId = Preview ? VisibleEntryIds.Find(Preview) : nullptr;
	const FInventoryEntry* Entry = Inventory && EntryId ? Inventory->FindEntry(*EntryId) : nullptr;
	const FItemRow* Definition = Entry ? Inventory->FindItemDefinition(Entry->ItemRowName) : nullptr;
	if (!Definition || !Entry)
	{
		ItemInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
		ItemNameText->SetText(FText::GetEmpty());
		ItemQuantityText->SetText(FText::GetEmpty());
		ItemStatValueText->SetText(FText::GetEmpty());
		ItemStatRow->SetVisibility(ESlateVisibility::Collapsed);
		ItemDescriptionText->SetText(FText::GetEmpty());
		return;
	}

	ItemNameText->SetText(Definition->DisplayName);
	RefreshItemStat(*Definition, *Entry);
	ItemQuantityText->SetVisibility(ESlateVisibility::HitTestInvisible);
	ItemQuantityText->SetText(FText::Format(
		NSLOCTEXT("Inventory", "SlotQuantity", "In this slot: {0}"), FText::AsNumber(Entry->Quantity)));
	ItemDescriptionText->SetText(Definition->Description.IsEmpty()
		? NSLOCTEXT("Inventory", "MissingDescription", "No description available.") : Definition->Description);
	if (ItemDescriptionScroll) ItemDescriptionScroll->ScrollToStart();
	ItemInfoPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}



void UInventoryWidget::RefreshItemStat(const FItemRow& Definition, const FInventoryEntry& Entry)
{
	if (!ItemStatRow || !ItemStatIcon || !ItemStatValueText) return;

	UTexture2D* Icon = nullptr;
	FText Value;
	switch (Definition.Category)
	{
	case EItemCategory::Weapon:
		Icon = WeaponStatIcon.Get();
		Value = FText::AsNumber(Definition.ResolveEquipmentPower(Entry.EquipmentPower));
		break;
	case EItemCategory::Shields:
		Icon = ShieldStatIcon.Get();
		Value = FText::AsNumber(Definition.ResolveEquipmentPower(Entry.EquipmentPower));
		break;
	case EItemCategory::Food:
		Icon = FoodStatIcon.Get();
		Value = FText::AsNumber(FMath::Max(0.f, Definition.HealthRestored));
		break;
	default:
		ItemStatRow->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	ItemStatIcon->SetBrushFromTexture(Icon, false);
	ItemStatValueText->SetText(Value);
	ItemStatRow->SetVisibility(ESlateVisibility::HitTestInvisible);
}

// void UInventoryWidget::HandleCloseButtonClicked()
// {
// 	AUIInventorySystemPlayerController* PlayerController = Cast<AUIInventorySystemPlayerController>(GetOwningPlayer());
// 	if (PlayerController)
// 	{
// 		PlayerController->HideInventory();
// 	}
// }

