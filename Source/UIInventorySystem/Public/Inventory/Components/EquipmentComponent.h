// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

class UInventoryComponent;
DECLARE_MULTICAST_DELEGATE(FEquipmentChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIINVENTORYSYSTEM_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEquipmentComponent();

	FGuid GetEquippedWeaponId() const { return EquippedWeaponId; }
	FGuid GetEquippedShieldId() const { return EquippedShieldId; }

	bool TryEquipItem(const FGuid& EntryId, FText& OutErrorMessage);

	FEquipmentChanged OnEquipmentChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	UPROPERTY(VisibleInstanceOnly, Category = "Equipment")
	FGuid EquippedWeaponId;
	UPROPERTY(VisibleInstanceOnly, Category = "Equipment")
	FGuid EquippedShieldId;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> InventoryComponent = nullptr;

	void HandleInventoryChanged();
};
