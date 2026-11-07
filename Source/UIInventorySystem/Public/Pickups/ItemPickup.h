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

protected:

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool TryCollect(AActor* Collector) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FDataTableRowHandle ItemRow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup",
		meta = (ClampMin = "1", UIMin = "1"))
	int32 Quantity = 1;

};
