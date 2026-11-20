// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/PickupBase.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "PickupAnimation.h"

// Sets default values
APickupBase::APickupBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

	if (bCollectOnOverlap)
	{
		TryInteract(OtherActor);
	}
}



bool APickupBase::TryCollect(AActor* Collector)
{
	return false;
}

bool APickupBase::TryInteract(AActor* Interactor)
{
	if (bCollected || IsActorBeingDestroyed() ||
		!IsValid(Interactor) || Interactor == this) return false;

	bCollected = true;

	if (!TryCollect(Interactor))
	{
		bCollected = false;
		return false;
	}
	SetActorEnableCollision(false);
	Destroy();

	return true;
}

FVector APickupBase::GetFocusPoint() const
{
	return IsValid(PickupMesh.Get()) ? PickupMesh->Bounds.Origin : GetActorLocation();
}

void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	if (PickupMesh)
	{
		InitialMeshTransform = PickupMesh->GetRelativeTransform();
		if (const UStaticMesh* Mesh = PickupMesh->GetStaticMesh())
		{
			LocalMeshCenter = Mesh->GetBounds().Origin;
		}
	}

	SetActorTickEnabled(bAnimatePickup && IsValid(PickupMesh.Get()));
}

void APickupBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Animate the pickup mesh if enabled
	if (!bAnimatePickup || !PickupMesh) return;

	SpinDegrees = FMath::Fmod(SpinDegrees + RotationSpeed * DeltaTime, 360.f);
	const float FullCycle = 2.f * PI;
	BobPhase = FMath::Fmod(BobPhase + (FullCycle * BobFrequency * DeltaTime), FullCycle);
	const float HeightOffset = BobHeight * FMath::Sin(BobPhase);

	const USceneComponent* Parent = PickupMesh->GetAttachParent();
	const FTransform RestWorld = Parent
		? InitialMeshTransform * Parent->GetComponentTransform() : InitialMeshTransform;
	const FTransform Animated = PickupAnimation::MakeTransform(
		RestWorld, LocalMeshCenter, SpinDegrees, HeightOffset);
	PickupMesh->SetWorldLocationAndRotation(Animated.GetLocation(), Animated.GetRotation());
}


