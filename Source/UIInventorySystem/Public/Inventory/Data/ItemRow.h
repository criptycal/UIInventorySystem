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

	// Base attack power used by weapon items; this does not apply combat damage.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Weapon",
		meta = (ClampMin = "0", UIMin = "0"))
	int32 AttackPower = 0;

	// Base shield guard strength; blocking gameplay is implemented separately.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Shield",
		meta = (ClampMin = "0", UIMin = "0"))
	int32 GuardPower = 0;

	// Equipment rolls once when created; food does not use this value.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Equipment",
		meta = (ClampMin = "0", ClampMax = "1000", UIMin = "0"))
	int32 PowerVariance = 3;

	bool HasEquipmentPower() const
	{
		return Category == EItemCategory::Weapon || Category == EItemCategory::Shields;
	}

	int32 GetBaseEquipmentPower() const
	{
		return HasEquipmentPower()
			? FMath::Max(0, Category == EItemCategory::Weapon ? AttackPower : GuardPower) : 0;
	}

	int32 ResolveEquipmentPower(int32 StoredPower) const
	{
		return HasEquipmentPower() ? (StoredPower >= 0 ? StoredPower : GetBaseEquipmentPower()) : 0;
	}

	int32 RollEquipmentPower() const
	{
		if (!HasEquipmentPower()) return 0;
		const int64 Base = GetBaseEquipmentPower();
		const int64 Variance = FMath::Clamp(PowerVariance, 0, 1000);
		const int32 Minimum = static_cast<int32>(FMath::Max<int64>(0, Base - Variance));
		const int32 Maximum = static_cast<int32>(FMath::Min<int64>(MAX_int32, Base + Variance));
		// Roll an offset to avoid overflow near MAX_int32.
		return Minimum + FMath::RandRange(0, Maximum - Minimum);
	}

	// HealthComponent uses hearts: 1 = a full heart, 0.5 = half a heart.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Food",
		meta = (ClampMin = "0.0", UIMin = "0.0"))
	float HealthRestored = 0.f;


};
