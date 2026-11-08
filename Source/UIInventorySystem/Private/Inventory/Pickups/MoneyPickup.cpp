// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Pickups/MoneyPickup.h"
#include "Inventory/Components/CurrencyComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Inventory/Data/MoneyRow.h"
#include "UIInventorySystem.h"

// Sets default values
AMoneyPickup::AMoneyPickup()
{
	InitializePickupComponents(TEXT("PickupCollision"));
	bCollectOnOverlap = true;
}

void AMoneyPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CurrencyAmount = 0;
	PickupMesh->SetStaticMesh(nullptr);

	if (!CurrencyRow.DataTable || CurrencyRow.RowName.IsNone()) return;

	if (CurrencyRow.DataTable->GetRowStruct() != FMoneyRow::StaticStruct())
	{
		UE_LOG(LogUIInventorySystem, Warning, TEXT("CurrencyRow is not of type FMoneyRow"));
		return;
	}

	const FMoneyRow* MoneyRow = CurrencyRow.GetRow<FMoneyRow>(TEXT("MoneyPickup Construction"));

	if (!MoneyRow) return;

	if (MoneyRow->Amount <= 0 || !IsValid(MoneyRow->Mesh.Get()))
	{
		UE_LOG(LogUIInventorySystem, Warning, TEXT("Currency MoneyRow needs a positive amount and a mesh %s"), *GetNameSafe(this));
		return;
	}
	PickupMesh->SetStaticMesh(MoneyRow->Mesh.Get());
	CurrencyAmount = MoneyRow->Amount;
}

bool AMoneyPickup::TryCollect(AActor* Collector)
{
	UCurrencyComponent* Currency = Collector->FindComponentByClass<UCurrencyComponent>();

	if(!Currency)
	{
		UE_LOG(LogUIInventorySystem, Warning, TEXT("Collector %s has no currency component"), *GetNameSafe(Collector));
		return false;
	}

	if (!Currency->TryAddCurrency(CurrencyAmount))
	{
		UE_LOG(LogUIInventorySystem, Warning, TEXT("Collector %s could not add currency"), *GetNameSafe(Collector));
		return false;
	}

	UE_LOG(LogUIInventorySystem, Log, TEXT("Collector %s collected %d currency"), *GetNameSafe(Collector), CurrencyAmount);

	return true;
}


