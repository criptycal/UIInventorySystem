// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "MoneyPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class UIINVENTORYSYSTEM_API AMoneyPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMoneyPickup();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<USphereComponent> PickupSphere;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	int32 CurrencyAmount = 0;

	bool bCollected = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pickup")
	FDataTableRowHandle CurrencyRow;

	virtual void OnConstruction(const FTransform& Transform) override;

	

};
