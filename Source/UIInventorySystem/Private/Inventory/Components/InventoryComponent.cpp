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

	int32 RemainingQuantity = Quantity;

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

	//Sample Weapons
	TryAddItem(TEXT("BlizzardRod"), 2);
	TryAddItem(TEXT("KnightsBroadsword"), 1);
	TryAddItem(TEXT("SpikedBokoClub"), 1);

	// Sample shields.
	TryAddItem(TEXT("Daybreaker"), 1);
	TryAddItem(TEXT("RoyalGuard"), 1);
	TryAddItem(TEXT("SteelLizalShield"), 1);

	// Sample food.
	TryAddItem(TEXT("Apple"), 23);
	TryAddItem(TEXT("FortifiedPumpkin"), 2);
	TryAddItem(TEXT("Hydromelon"), 2);
	TryAddItem(TEXT("MightyBananas"), 3);
	TryAddItem(TEXT("SpicyPepper"), 5);

	UE_LOG(LogUIInventorySystem, Warning,
		TEXT("Sample inventory contains %d entries."),
		Entries.Num());
}
