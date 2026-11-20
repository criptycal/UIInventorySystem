// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "Engine/DataTable.h"
#include "ItemPickup.generated.h"

UCLASS()
class UIINVENTORYSYSTEM_API AItemPickup : public APickupBase
{
	GENERATED_BODY()

public:
	AItemPickup();

	void SetHighlighted(bool bHighlighted);

	// Configure between SpawnActorDeferred and FinishSpawning (before construction/BeginPlay).
	void InitializeDroppedItem(UDataTable* Definitions, FName RowName, int32 InEquipmentPower);

	int32 GetEquipmentPower() const { return EquipmentPower; }

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool TryCollect(AActor* Collector) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FDataTableRowHandle ItemRow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup",
		meta = (ClampMin = "1", UIMin = "1"))
	int32 Quantity = 1;

private:
	// Generated once in play, never rerolled by construction, focus, or failed collection.
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Pickup")
	int32 EquipmentPower = INDEX_NONE;

	bool EnsureEquipmentPower();

};
