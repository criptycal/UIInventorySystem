#include "Inventory/Components/InventoryComponent.h"

#include "Inventory/Components/HealthComponent.h"
#include "Inventory/Data/ItemRow.h"
#include "Pickups/ItemPickup.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"

namespace
{
	// Try a few nearby places in front of the character. Sweep the mesh bounds so
	// the drop cannot pass through a wall or overlap another blocking object.
	bool FindDropTransform(AActor& Owner, const UStaticMesh& Mesh, FTransform& OutTransform)
	{
		UWorld* World = Owner.GetWorld();
		if (!World) return false;
		const FBoxSphereBounds Bounds = Mesh.GetBounds();
		const FVector Extent = Bounds.BoxExtent.ComponentMax(FVector(2.f));
		const FCollisionShape Shape = FCollisionShape::MakeBox(Extent);
		FCollisionQueryParams Query(SCENE_QUERY_STAT(InventoryDrop), false, &Owner);
		const FVector Start = Owner.GetActorLocation();
		const float Distance = 100.f + FMath::Max(Extent.X, Extent.Y);
		for (const float Angle : { 0.f, 35.f, -35.f, 70.f, -70.f })
		{
			const FVector Direction = FRotator(0.f, Owner.GetActorRotation().Yaw + Angle, 0.f).Vector();
			const FVector Candidate = Start + Direction * Distance;
			FHitResult Ground;
			if (!World->LineTraceSingleByChannel(Ground, Candidate + FVector(0, 0, 100),
				Candidate - FVector(0, 0, 400), ECC_Visibility, Query)
				|| Ground.ImpactNormal.Z < 0.7f) continue;

			const FVector Center = Ground.ImpactPoint + FVector(0, 0, Extent.Z + 3.f);
			FHitResult Obstacle;
			if (World->SweepSingleByChannel(Obstacle, Start, Center, FQuat::Identity,
				ECC_Visibility, Shape, Query)) continue;
			if (World->OverlapBlockingTestByChannel(Center, FQuat::Identity,
				ECC_Visibility, Shape, Query)) continue;

			OutTransform = FTransform(FQuat::Identity, Center - Bounds.Origin);
			return true;
		}
		return false;
	}
}

const FInventoryEntry* UInventoryComponent::FindEntry(const FGuid& EntryId) const
{
	return Entries.FindByPredicate([&EntryId](const FInventoryEntry& Entry)
	{
		return Entry.EntryId == EntryId && Entry.Quantity > 0;
	});
}

bool UInventoryComponent::RemoveOne(const FGuid& EntryId)
{
	const int32 Index = Entries.IndexOfByPredicate([&EntryId](const FInventoryEntry& Entry)
	{
		return Entry.EntryId == EntryId && Entry.Quantity > 0;
	});
	if (Index == INDEX_NONE) return false;
	if (--Entries[Index].Quantity == 0) Entries.RemoveAt(Index);
	return true;
}

bool UInventoryComponent::TryDeleteItem(const FGuid& EntryId, FText& OutFailure)
{
	OutFailure = FText::GetEmpty();
	if (!RemoveOne(EntryId))
	{
		OutFailure = NSLOCTEXT("Inventory", "MissingActionItem", "This item is no longer in your inventory.");
		return false;
	}
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::TryEatItem(const FGuid& EntryId, FText& OutFailure)
{
	OutFailure = FText::GetEmpty();
	const FInventoryEntry* Entry = FindEntry(EntryId);
	const FItemRow* Definition = Entry ? FindItemDefinition(Entry->ItemRowName) : nullptr;
	UHealthComponent* Health = GetOwner() ? GetOwner()->FindComponentByClass<UHealthComponent>() : nullptr;
	if (!Definition || Definition->Category != EItemCategory::Food || !Health
		|| !FMath::IsFinite(Definition->HealthRestored) || Definition->HealthRestored <= 0.f)
	{
		OutFailure = NSLOCTEXT("Inventory", "CannotEat", "This item cannot restore health.");
		return false;
	}
	if (Health->GetCurrentHealth() >= Health->GetMaxHealth())
	{
		OutFailure = NSLOCTEXT("Inventory", "AlreadyHealthy", "Your health is already full.");
		return false;
	}
	const float Healing = Definition->HealthRestored;
	// Remove before Heal broadcasts to listeners, so they see the consumed item.
	RemoveOne(EntryId);
	Health->Heal(Healing);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::TryDropItem(const FGuid& EntryId, FText& OutFailure)
{
	OutFailure = FText::GetEmpty();
	const FInventoryEntry* Entry = FindEntry(EntryId);
	const FItemRow* Definition = Entry ? FindItemDefinition(Entry->ItemRowName) : nullptr;
	AActor* Owner = GetOwner();
	if (!Definition || !Definition->WorldMesh || !Owner || !GetWorld())
	{
		OutFailure = NSLOCTEXT("Inventory", "CannotDrop", "This item cannot be placed in the world.");
		return false;
	}
	FTransform Transform;
	if (!FindDropTransform(*Owner, *Definition->WorldMesh, Transform))
	{
		OutFailure = NSLOCTEXT("Inventory", "NoDropSpace", "No clear space nearby. Move away from obstacles and try again.");
		return false;
	}
	const FName RowName = Entry->ItemRowName;
	const int32 Power = Definition->ResolveEquipmentPower(Entry->EquipmentPower);
	AItemPickup* Pickup = GetWorld()->SpawnActorDeferred<AItemPickup>(AItemPickup::StaticClass(),
		Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Pickup)
	{
		OutFailure = NSLOCTEXT("Inventory", "DropSpawnFailed", "Could not drop this item. It remains in your inventory.");
		return false;
	}
	Pickup->InitializeDroppedItem(ItemDefinitions, RowName, Power);
	Pickup->FinishSpawning(Transform);
	if (!IsValid(Pickup) || Pickup->IsActorBeingDestroyed() || !RemoveOne(EntryId))
	{
		if (IsValid(Pickup)) Pickup->Destroy();
		OutFailure = NSLOCTEXT("Inventory", "DropSpawnFailed", "Could not drop this item. It remains in your inventory.");
		return false;
	}
	OnInventoryChanged.Broadcast();
	return true;
}
