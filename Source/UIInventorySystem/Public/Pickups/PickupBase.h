// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS(Abstract)
class UIINVENTORYSYSTEM_API APickupBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickupBase();

	virtual void Tick(float DeltaTime) override;

protected:
	void InitializePickupComponents(FName CollisionComponentName);

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	virtual bool TryCollect(AActor* Collector);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Animation")
	float RotationSpeed = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Animation",
		meta = (ClampMin = "0.0", UIMin = "0.0"))
	float BobHeight = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup|Animation",
		meta = (ClampMin = "0.0", UIMin = "0.0"))
	float BobFrequency = 0.5f;

private:
	bool bCollected = false;

	FVector InitialMeshLocation = FVector::ZeroVector;
	float BobPhase = 0.f;

};
