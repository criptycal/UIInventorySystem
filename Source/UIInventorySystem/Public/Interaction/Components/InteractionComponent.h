// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class AItemPickup;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusedItemChanged, AItemPickup*, NewFocusedItem);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIINVENTORYSYSTEM_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInteractionComponent();

	void Interact();

	AItemPickup* GetFocusedItem() const;


	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	FOnFocusedItemChanged OnFocusedItemChanged;

	void CycleFocusedItem(int32 Direction);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void UpdateFocus();

	//
	AItemPickup* FindFocusedItem(
		TArray<AItemPickup*>* OutCandidates = nullptr // Optional output parameter to receive the list of candidate items
	) const;

	void SetFocusedItem(AItemPickup* NewItem);

	UPROPERTY(EditAnywhere, Category = "Interaction",
		meta = (ClampMin = "1.0", UIMin = "1.0"))
	float InteractionDistance = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction",
		meta = (ClampMin = "1.0", UIMin = "1.0"))
	float TraceDistance = 1000.0f;

	UPROPERTY(Transient)
	TWeakObjectPtr<AItemPickup> FocusedItem;

	TArray<AItemPickup*> GatherNearbyItems() const;

	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "0.0", ClampMax = "0.25"))
	float FocusRadiusScreenFraction = 0.06f;

	UPROPERTY(Transient)
	TWeakObjectPtr<AItemPickup> ManuallySelectedItem;

};
