// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/PickupBase.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
APickupBase::APickupBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void APickupBase::InitializePickupComponents(FName CollisionComponentName)
{
	PickupSphere = CreateDefaultSubobject<USphereComponent>(CollisionComponentName);
	PickupSphere->InitSphereRadius(75.0f);
	PickupSphere->SetGenerateOverlapEvents(true);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SetRootComponent(PickupSphere);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetGenerateOverlapEvents(false);
	PickupMesh->SetupAttachment(PickupSphere);
}

void APickupBase::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (bCollected || !IsValid(OtherActor) || OtherActor == this) return;
	bCollected = true;

	if (!TryCollect(OtherActor))
	{
		bCollected = false;
		return;
	}

	SetActorEnableCollision(false);
	Destroy();
}

bool APickupBase::TryCollect(AActor* Collector)
{
	return false;
}
