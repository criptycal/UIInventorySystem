// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Pickups/MoneyPickup.h"
#include "Components/SphereComponent.h"
#include "Inventory/Components/CurrencyComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Inventory/Data/MoneyRow.h"

// Sets default values
AMoneyPickup::AMoneyPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
	SetRootComponent(PickupSphere);
	PickupSphere->InitSphereRadius(75.0f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	PickupSphere->SetGenerateOverlapEvents(true);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(PickupSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetGenerateOverlapEvents(false);

}

void AMoneyPickup::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (bCollected || !IsValid(OtherActor) || OtherActor == this) return;
	UE_LOG(LogTemp, Warning, TEXT("Money Pickup Overlapped with: %s"), *GetNameSafe(OtherActor));

	UCurrencyComponent* CurrencyComp = OtherActor->FindComponentByClass<UCurrencyComponent>();
	if (!CurrencyComp) return;
	bCollected = true;
	
	if (!CurrencyComp->TryAddCurrency(CurrencyAmount))
	{
		bCollected = false;
		UE_LOG(LogTemp, Warning, TEXT("Failed to add currency to %s"), *GetNameSafe(OtherActor));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Added %d currency to %s"), CurrencyAmount, *GetNameSafe(OtherActor));

	SetActorEnableCollision(false);
	Destroy();
}

void AMoneyPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	CurrencyAmount = 0;
	PickupMesh->SetStaticMesh(nullptr);

	if (!CurrencyRow.DataTable || CurrencyRow.RowName.IsNone()) return;

	if (CurrencyRow.DataTable->GetRowStruct() != FMoneyRow::StaticStruct())
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrencyRow is not of type FMoneyRow"));
		return;
	}

	const FMoneyRow* MoneyRow = CurrencyRow.GetRow<FMoneyRow>(TEXT("MoneyPickup Construction"));

	if (!MoneyRow) return;

	if (MoneyRow->Amount <= 0 || !IsValid(MoneyRow->Mesh.Get()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Currency MoneyRow needs a positive amount and a mesh %s"), *GetNameSafe(this));
		return;
	}
	PickupMesh->SetStaticMesh(MoneyRow->Mesh.Get());
	CurrencyAmount = MoneyRow->Amount;
}


