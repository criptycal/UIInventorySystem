#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class AItemPickup;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFocusedItemChanged, AItemPickup*, NewFocusedItem);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UIINVENTORYSYSTEM_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	void Interact();
	void CycleFocusedItem(int32 Direction);
	AItemPickup* GetFocusedItem() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	FOnFocusedItemChanged OnFocusedItemChanged;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Temporary query data, defined in the .cpp; never stored across frames.
	struct FFocusContext;
	struct FFocusCandidate;

	bool BuildFocusContext(FFocusContext& OutContext) const;
	bool CanReachItem(const FFocusContext& Context, AItemPickup* Item, const FVector& HitPoint) const;
	AItemPickup* FindDirectTarget(const FFocusContext& Context) const;
	TArray<AItemPickup*> GatherNearbyItems(const FFocusContext& Context) const;
	FFocusCandidate MakeFocusCandidate(const FFocusContext& Context, AItemPickup* Item) const;
	bool EvaluateCandidate(const FFocusContext& Context, AItemPickup* Item, double ScoreLimit, FFocusCandidate& OutCandidate) const;

	// A caller may request the full eligible list for manual selection/cycling.
	AItemPickup* FindFocusedItem(TArray<FFocusCandidate>* OutCandidates = nullptr) const;
	static int32 FindCandidateIndex(const TArray<FFocusCandidate>& Candidates, AItemPickup* Item);
	int32 FindCycleStartIndex(const TArray<FFocusCandidate>& Candidates, AItemPickup* AutomaticItem) const;
	void UpdateFocus();
	void SetFocusedItem(AItemPickup* NewItem);

	UPROPERTY(EditAnywhere, Category = "Interaction",
		meta = (ClampMin = "1.0", UIMin = "1.0"))
	float InteractionDistance = 200.0f;

	// Horizontal angle allowed on either side of the character's forward direction.
	UPROPERTY(EditAnywhere, Category = "Interaction",
		meta = (ClampMin = "0.0", ClampMax = "90.0", Units = "Degrees"))
	float InteractionHalfAngleDegrees = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction",
		meta = (ClampMin = "1.0", UIMin = "1.0"))
	float TraceDistance = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction",
		meta = (ClampMin = "0.0", ClampMax = "0.25"))
	float FocusRadiusScreenFraction = 0.06f;

	UPROPERTY(Transient)
	TWeakObjectPtr<AItemPickup> FocusedItem;

	UPROPERTY(Transient)
	TWeakObjectPtr<AItemPickup> ManuallySelectedItem;
};
