#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Inventory/Data/ItemCategory.h"
#include "ItemRow.generated.h"

class UTexture2D;
class UStaticMesh;

USTRUCT(BlueprintType)
struct UIINVENTORYSYSTEM_API FItemRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
		meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EItemCategory Category = EItemCategory::Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UStaticMesh> WorldMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
		meta = (ClampMin = "1", UIMin = "1"))
	int32 MaxStackSize = 1;

	// The maximum number of this item that a player can own in total, across all stacks.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item",
		meta = (ClampMin = "0", UIMin = "0"))
	int32 MaxOwnedQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int32 SortPriority = 0;


};