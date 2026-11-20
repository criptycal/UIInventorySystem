// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Components/EquipmentComponent.h"
#include "Inventory/Components/InventoryComponent.h"
#include "GameFramework/Actor.h"
#include "Inventory/Data/ItemRow.h"

// Sets default values for this component's properties
UEquipmentComponent::UEquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	InventoryComponent = Owner->FindComponentByClass<UInventoryComponent>();

	if (!InventoryComponent)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("EquipmentComponent on %s requires an InventoryComponent."),
			*Owner->GetName());
		return;
	}

	InventoryComponent->OnInventoryChanged.AddUObject(this, &UEquipmentComponent::HandleInventoryChanged);

	HandleInventoryChanged();
}

void UEquipmentComponent::HandleInventoryChanged()
{
	if (!IsValid(InventoryComponent.Get())) return;

	bool bEquipmentChanged = false;

	if(EquippedWeaponId.IsValid() &&
		!InventoryComponent->FindEntry(EquippedWeaponId))
	{
		EquippedWeaponId.Invalidate();
		bEquipmentChanged = true;
	}

	if (EquippedShieldId.IsValid() &&
		!InventoryComponent->FindEntry(EquippedShieldId))
	{
		EquippedShieldId.Invalidate();
		bEquipmentChanged = true;
	}
	if (bEquipmentChanged)
	{
		OnEquipmentChanged.Broadcast();
	}
}

bool UEquipmentComponent::TryEquipItem(const FGuid& EntryId, FText& OutErrorMessage)
{
	OutErrorMessage = FText::GetEmpty();

	if (!IsValid(InventoryComponent.Get()))
	{
		OutErrorMessage = NSLOCTEXT(
			"EquipmentComponent", "MissingInventory", "Inventory is unavailable.");
		return false;
	}

	const FInventoryEntry* Entry = InventoryComponent->FindEntry(EntryId);

	if (!Entry || Entry->Quantity <= 0)
	{
		OutErrorMessage = NSLOCTEXT("EquipmentComponent", "ItemNotFoundOrQuantityZero", "Item not found in inventory or quantity is zero.");
		return false;
	}

	const FItemRow* Definition =
		InventoryComponent->FindItemDefinition(Entry->ItemRowName);

	if (!Definition)
	{
		OutErrorMessage = NSLOCTEXT("EquipmentComponent", "ItemDefinitionNotFound", "Item definition not found.");
		return false;
	}

	switch (Definition->Category)
	{
	case EItemCategory::Weapon:
		if (EquippedWeaponId == Entry->EntryId)
		{
			return true;
		}

		EquippedWeaponId = Entry->EntryId;
		break;

	case EItemCategory::Shields:
		if (EquippedShieldId == Entry->EntryId)
		{
			return true;
		}

		EquippedShieldId = Entry->EntryId;
		break;

	default:
		OutErrorMessage = NSLOCTEXT(
			"EquipmentComponent", "CannotEquip",
			"This item cannot be equipped.");
		return false;
	}
	OnEquipmentChanged.Broadcast();
	return true;
}

void UEquipmentComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(InventoryComponent.Get()))
	{
		InventoryComponent->OnInventoryChanged.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}
