#include "UI/InventoryWidget.h"

#include "UI/ItemActionMenuWidget.h"
#include "UI/ItemSlotWidget.h"
#include "Inventory/Components/InventoryComponent.h"
#include "Inventory/Data/ItemRow.h"
#include "UIInventorySystemPlayerController.h"
#include "Inventory/Components/EquipmentComponent.h"
#include "GameFramework/Pawn.h"

void UInventoryWidget::OpenActionMenu()
{
	if (ActionMenu || !ObservedInventory.IsValid()) return;
	const FGuid* EntryId = VisibleEntryIds.Find(SelectedSlot);
	const FInventoryEntry* Entry = EntryId ? ObservedInventory->FindEntry(*EntryId) : nullptr;
	const FItemRow* Definition = Entry ? ObservedInventory->FindItemDefinition(Entry->ItemRowName) : nullptr;
	if (!Definition) return;

	ActionEntryId = *EntryId;
	HoveredSlot.Reset();
	RefreshItemInfo();
	ActionMenu = CreateWidget<UItemActionMenuWidget>(GetOwningPlayer(), UItemActionMenuWidget::StaticClass());
	if (!ActionMenu) return;
	ActionMenu->Configure(Definition->DisplayName, Definition->Category == EItemCategory::Food);
	ActionMenu->SetAnchorSlot(SelectedSlot.Get());
	ActionMenu->OnActionRequested.BindUObject(this, &UInventoryWidget::HandleItemAction);
	ActionMenu->OnDismissed.BindUObject(this, &UInventoryWidget::HandleMenuDismissed);
	ActionMenu->AddToPlayerScreen(20);
	ActionMenu->SetKeyboardFocus();
}

void UInventoryWidget::CloseActionMenu()
{
	if (ActionMenu)
	{
		ActionMenu->OnActionRequested.Unbind();
		ActionMenu->OnDismissed.Unbind();
		ActionMenu->RemoveFromParent();
		ActionMenu = nullptr;
	}
	ActionEntryId.Invalidate();
}

void UInventoryWidget::HandleMenuDismissed(bool bCloseInventory)
{
	CloseActionMenu();
	if (bCloseInventory)
	{
		if (auto* Controller = Cast<AUIInventorySystemPlayerController>(GetOwningPlayer())) Controller->HideInventory();
	}
	else SetKeyboardFocus();
}

void UInventoryWidget::HandleInventoryChanged()
{
	RebuildItemGrid(); // Closes any stale menu, clears selection and clamps the page.
	SetKeyboardFocus();
}

void UInventoryWidget::HandleItemAction(EInventoryItemAction Action)
{
	if (!ActionMenu || !ObservedInventory.IsValid()) return;
	FText Failure;
	// Copy the ID: success broadcasts a refresh that invalidates ActionEntryId.
	const FGuid EntryId = ActionEntryId;
	bool bSucceeded = false;
	switch (Action)
	{
	case EInventoryItemAction::Equip:
	{
		APawn* PlayerPawn = GetOwningPlayerPawn();
		UEquipmentComponent* Equipment = IsValid(PlayerPawn)
			? PlayerPawn->FindComponentByClass<UEquipmentComponent>()
			: nullptr;

		if (IsValid(Equipment))
		{
			bSucceeded = Equipment->TryEquipItem(EntryId, Failure);
		}
		else
		{
			Failure = NSLOCTEXT("Inventory", "NoEquipmentComponent", "No equipment component found.");
		}

		if (bSucceeded)
		{
			CloseActionMenu();
			SetKeyboardFocus();
		}

		break;
	}
	case EInventoryItemAction::Eat:
		bSucceeded = ObservedInventory->TryEatItem(EntryId, Failure);
		break;
	case EInventoryItemAction::Drop:
		bSucceeded = ObservedInventory->TryDropItem(EntryId, Failure);
		break;
	case EInventoryItemAction::Delete:
		bSucceeded = ObservedInventory->TryDeleteItem(EntryId, Failure);
		break;
	}
	if (!bSucceeded && ActionMenu) ActionMenu->ShowFailure(Failure);
}
