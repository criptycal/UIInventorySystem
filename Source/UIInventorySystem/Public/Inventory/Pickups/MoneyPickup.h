// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "Engine/DataTable.h"
#include "MoneyPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class UIINVENTORYSYSTEM_API AMoneyPickup : public APickupBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMoneyPickup();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual bool TryCollect(AActor* Collector) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	int32 CurrencyAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FDataTableRowHandle CurrencyRow;
};
