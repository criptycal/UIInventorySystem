#include "Pickups/ItemPickup.h"

#include "Components/StaticMeshComponent.h"
#include "Inventory/Data/ItemRow.h"
#include "UIInventorySystem.h"
#include "Inventory/Components/InventoryComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AItemPickup::AItemPickup()
{
	InitializePickupComponents(TEXT("PickupSphere"));
	
	// Set default values for the pickup mesh and sphere
	PickupMesh->SetCustomDepthStencilValue(1);
	PickupMesh->SetRenderCustomDepth(false);
	PickupSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

}

void AItemPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	PickupMesh->SetStaticMesh(nullptr);

	if (!ItemRow.DataTable || ItemRow.RowName.IsNone())
	{
		return;
	}

	if (ItemRow.DataTable->GetRowStruct() != FItemRow::StaticStruct())
	{
		UE_LOG(LogUIInventorySystem, Warning,
			TEXT("%s: Select a data table using FItemRow."),
			*GetNameSafe(this));
		return;
	}

	const FItemRow* Definition = ItemRow.DataTable->FindRow<FItemRow>(
		ItemRow.RowName, TEXT("ItemPickup::OnConstruction"), false);

	if (!Definition || !IsValid(Definition->WorldMesh.Get()))
	{
		UE_LOG(LogUIInventorySystem, Warning,
			TEXT("%s: Item row '%s' is missing or has no world mesh."),
			*GetNameSafe(this), *ItemRow.RowName.ToString());
		return;
	}

	PickupMesh->SetStaticMesh(Definition->WorldMesh.Get());
}

bool AItemPickup::TryCollect(AActor* Collector)
{
	UInventoryComponent* Inventory = Collector->FindComponentByClass<UInventoryComponent>();

	if (!Inventory)
	{
		UE_LOG(LogUIInventorySystem, Warning,
			TEXT("%s: Collector '%s' has no inventory component."),
			*GetNameSafe(this), *GetNameSafe(Collector));
		return false;
	}

	if (!ItemRow.DataTable || ItemRow.DataTable != Inventory->GetItemDefinitions())
	{
		UE_LOG(LogUIInventorySystem, Warning,
			TEXT("%s: Pickup and inventory must use the same item table."),
			*GetNameSafe(this));
		return false;
	}

	if (!Inventory->TryAddItem(ItemRow.RowName, Quantity))
	{
		UE_LOG(LogUIInventorySystem, Warning,
			TEXT("%s: Collector '%s' could not add item '%s' to inventory."),
			*GetNameSafe(this), *GetNameSafe(Collector), *ItemRow.RowName.ToString());
		return false;
	}

	UE_LOG(LogUIInventorySystem, Log,
		TEXT("%s: Collector '%s' collected item '%s' (x%d)."),
		*GetNameSafe(this), *GetNameSafe(Collector), *ItemRow.RowName.ToString(), Quantity);

	return true;
}

void AItemPickup::SetHighlighted(bool bHighlighted)
{
	if (PickupMesh)
	{
		PickupMesh->SetRenderCustomDepth(bHighlighted);
	}
}
