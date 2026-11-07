#pragma once

#include "CoreMinimal.h"
#include "InventoryEntry.generated.h"

USTRUCT(BlueprintType)
struct UIINVENTORYSYSTEM_API FInventoryEntry
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FName ItemRowName = NAME_None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 Quantity = 1;
};