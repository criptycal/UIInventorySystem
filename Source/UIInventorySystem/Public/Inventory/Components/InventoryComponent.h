// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Data/InventoryEntry.h"
#include "InventoryComponent.generated.h"

struct FItemRow;
class UDataTable;

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

	const FItemRow* FindItemDefinition(FName ItemRowName) const;

	const UDataTable* GetItemDefinitions() const { return ItemDefinitions.Get(); }

private:
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UDataTable> ItemDefinitions = nullptr;

	UPROPERTY(VisibleInstanceOnly, Category = "Inventory")
	TArray<FInventoryEntry> Entries;

	UPROPERTY(EditDefaultsOnly, Category="Inventory|Testing")
	bool bAddSampleItems = false;

protected:
	virtual void BeginPlay() override;

};
