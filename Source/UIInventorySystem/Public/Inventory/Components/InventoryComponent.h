// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Data/InventoryEntry.h"
#include "Inventory/Data/ItemCategory.h"
#include "InventoryComponent.generated.h"

struct FItemRow;
class UDataTable;

DECLARE_MULTICAST_DELEGATE(FInventoryChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIINVENTORYSYSTEM_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventoryComponent();
	const TArray<FInventoryEntry>& GetEntries() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryAddItem(FName ItemRowName, int32 Quantity);

	// Adds a batch of identical copies; pickups use their persistent roll here.
	bool TryAddItemWithPower(FName ItemRowName, int32 Quantity, int32 EquipmentPower);

	const FItemRow* FindItemDefinition(FName ItemRowName) const;

	const UDataTable* GetItemDefinitions() const { return ItemDefinitions.Get(); }

	int32 GetSlotCapacity(EItemCategory Category) const;

	const FInventoryEntry* FindEntry(const FGuid& EntryId) const;
	bool TryEatItem(const FGuid& EntryId, FText& OutFailure);
	bool TryDropItem(const FGuid& EntryId, FText& OutFailure);
	bool TryDeleteItem(const FGuid& EntryId, FText& OutFailure);
	FInventoryChanged OnInventoryChanged;

private:
	bool RemoveOne(const FGuid& EntryId);
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UDataTable> ItemDefinitions = nullptr;

	UPROPERTY(VisibleInstanceOnly, Category = "Inventory")
	TArray<FInventoryEntry> Entries;

	UPROPERTY(EditDefaultsOnly, Category="Inventory|Testing")
	bool bAddSampleItems = false;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Capacity",
		meta = (ClampMin = "0", UIMin = "0"))
	int32 WeaponSlotCapacity = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Capacity",
		meta = (ClampMin = "0", UIMin = "0"))
	int32 ShieldsSlotCapacity = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Capacity",
		meta = (ClampMin = "0", UIMin = "0"))
	int32 FoodSlotCapacity = 10;

protected:
	virtual void BeginPlay() override;

};
