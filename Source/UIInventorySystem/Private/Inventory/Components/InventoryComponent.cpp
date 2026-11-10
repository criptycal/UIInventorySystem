// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Components/InventoryComponent.h"
#include "Engine/DataTable.h"
#include "Inventory/Data/ItemRow.h"
#include "UIInventorySystem.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

const TArray<FInventoryEntry>& UInventoryComponent::GetEntries() const
{
	return Entries;
}

bool UInventoryComponent::TryAddItem(FName ItemRowName, int32 Quantity)
{
	if (ItemRowName.IsNone() || Quantity <= 0)
	{
		UE_LOG(LogUIInventorySystem, Warning, TEXT("Cannot add item: invalid row name or quantity"));
		return false;
	}

	if (!ItemDefinitions || ItemDefinitions->GetRowStruct() != FItemRow::StaticStruct())
	{
		UE_LOG(LogUIInventorySystem, Warning, TEXT("Cannot add item: ItemDefinitions DataTable is not set or has the wrong row struct"));
		return false;
	}

	const FItemRow* Definition = ItemDefinitions->FindRow<FItemRow>(ItemRowName, TEXT("InventoryComponent::TryAddItem"), false);

	if (!Definition)
	{
		UE_LOG(LogUIInventorySystem, Warning, TEXT("Cannot add item: ItemDefinitions DataTable does not contain row '%s'"), *ItemRowName.ToString());
		return false;
	}

	const int32 MaxStackSize = Definition->MaxStackSize;

	if (MaxStackSize <= 0)
	{
		UE_LOG(LogUIInventorySystem, Warning, TEXT("Cannot add item: ItemDefinitions DataTable row '%s' has invalid MaxStackSize (%d)"), *ItemRowName.ToString(), MaxStackSize);
		return false;
	}

	int64 OwnedQuantity = 0;

	int32 RemainingQuantity = Quantity;

	const int32 SlotCapacity = GetSlotCapacity(Definition->Category);

	int32 UsedCategorySlots = 0;

	int32 QuantityNeedingNewSlots = Quantity;

	for (const FInventoryEntry& Entry : Entries)
	{
		const FItemRow* ExistingDefinition =
			FindItemDefinition(Entry.ItemRowName);

		if (!ExistingDefinition)
		{
			UE_LOG(LogUIInventorySystem, Warning,
				TEXT("InventoryComponent::TryAddItem: Existing entry '%s' has no valid definition"),
				*Entry.ItemRowName.ToString());
			return false;
		}

		if (ExistingDefinition->Category == Definition->Category)
		{
			++UsedCategorySlots;
		}

		if (Entry.ItemRowName == ItemRowName)
		{
			OwnedQuantity += Entry.Quantity;
			const int32 SpaceInStack =
				FMath::Max(0, MaxStackSize - Entry.Quantity);

			const int32 QuantityThatFits = FMath::Min(SpaceInStack, QuantityNeedingNewSlots);

			QuantityNeedingNewSlots -= QuantityThatFits;
		}
	}

	if (Definition->MaxOwnedQuantity > 0 && OwnedQuantity + Quantity > Definition->MaxOwnedQuantity)
	{
		UE_LOG(LogUIInventorySystem, Warning,
			TEXT("Cannot add '%s' x%d: would exceed MaxOwnedQuantity (%d)"),
			*ItemRowName.ToString(),
			Quantity,
			Definition->MaxOwnedQuantity);
		return false;
	}

	const int32 FreeCategorySlots = FMath::Max(0, SlotCapacity - UsedCategorySlots);

	const int32 RequiredNewSlots = QuantityNeedingNewSlots > 0
		? 1 + (QuantityNeedingNewSlots - 1) / MaxStackSize
		: 0;

	// Check if there are enough free slots for the new item stacks
	if (RequiredNewSlots > FreeCategorySlots)
	{
		UE_LOG(LogUIInventorySystem, Warning,
			TEXT("Cannot add '%s' x%d: needs %d new slots, but only %d are free."),
			*ItemRowName.ToString(),
			Quantity,
			RequiredNewSlots,
			FreeCategorySlots);
		return false;
	}

	// Fill existing stacks before creating new ones
	for (FInventoryEntry& Entry : Entries)
	{
		if (Entry.ItemRowName != ItemRowName ||
			Entry.Quantity >= MaxStackSize)
		{
			continue;
		}

		const int32 AvailableSpace = MaxStackSize - Entry.Quantity;
		const int32 AmountToAdd = FMath::Min(AvailableSpace, RemainingQuantity);

		Entry.Quantity += AmountToAdd;
		RemainingQuantity -= AmountToAdd;

		if (RemainingQuantity <= 0)
		{
			return true; // All items added successfully
		}
	}

	//split the remaining quantity into new stacks
	while (RemainingQuantity > 0)
	{
		const int32 StackQuantity = FMath::Min(MaxStackSize, RemainingQuantity);
		FInventoryEntry NewEntry;
		NewEntry.ItemRowName = ItemRowName;
		NewEntry.Quantity = StackQuantity;

		Entries.Add(NewEntry);
		RemainingQuantity -= StackQuantity;
	}
	return true;
}

const FItemRow* UInventoryComponent::FindItemDefinition(
	FName ItemRowName) const
{
	if (ItemRowName.IsNone() || !ItemDefinitions)
	{
		return nullptr;
	}

	if (ItemDefinitions->GetRowStruct() != FItemRow::StaticStruct())
	{
		return nullptr;
	}

	return ItemDefinitions->FindRow<FItemRow>(
		ItemRowName,
		TEXT("InventoryComponent::FindItemDefinition"),
		false
	);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!bAddSampleItems) return;

	// Sample weapons: 36 separate entries for pagination testing.
	TryAddItem(TEXT("BlizzardRod"), 12);
	TryAddItem(TEXT("KnightsBroadsword"), 12);
	TryAddItem(TEXT("SpikedBokoClub"), 12);

	// Sample shields: 25 entries; SteelLizalShield has an ownership limit of one.
	TryAddItem(TEXT("Daybreaker"), 12);
	TryAddItem(TEXT("RoyalGuard"), 12);
	TryAddItem(TEXT("SteelLizalShield"), 1);

	// Sample food: with the current 999 stack sizes, each 5000 creates six
	// stacks (five full stacks and one with five items). Plus apples = 25 entries.
	TryAddItem(TEXT("Apple"), 23);
	TryAddItem(TEXT("FortifiedPumpkin"), 5000);
	TryAddItem(TEXT("Hydromelon"), 5000);
	TryAddItem(TEXT("MightyBananas"), 5000);
	TryAddItem(TEXT("SpicyPepper"), 5000);

	UE_LOG(LogUIInventorySystem, Warning,
		TEXT("Sample inventory contains %d entries."),
		Entries.Num());
}

int32 UInventoryComponent::GetSlotCapacity(EItemCategory Category) const
{
	switch (Category)
	{
	case EItemCategory::Weapon:
		return FMath::Max(0, WeaponSlotCapacity);
	case EItemCategory::Shields:
		return FMath::Max(0, ShieldsSlotCapacity);
	case EItemCategory::Food:
		return FMath::Max(0, FoodSlotCapacity);
	default:
		UE_LOG(LogUIInventorySystem, Warning, TEXT("GetSlotCapacity: Unknown category %d"), static_cast<int32>(Category));
		return 0;
	}
}
