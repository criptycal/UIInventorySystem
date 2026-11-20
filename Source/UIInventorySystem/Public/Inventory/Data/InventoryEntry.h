#pragma once

#include "CoreMinimal.h"
#include "InventoryEntry.generated.h"

USTRUCT(BlueprintType)
struct UIINVENTORYSYSTEM_API FInventoryEntry
{
	GENERATED_BODY()
	// Identifies this particular stack/copy, independently of sorting and row name.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FGuid EntryId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FName ItemRowName = NAME_None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 Quantity = 1;

	// All copies in this stack share this roll. INDEX_NONE identifies older entries.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 EquipmentPower = INDEX_NONE;
};
