#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Inventory/Components/InventoryComponent.h"
#include "Inventory/Data/ItemRow.h"
#include "Pickups/ItemPickup.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryEquipmentTest,
	"UIInventorySystem.Inventory.EquipmentStacks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryEquipmentTest::RunTest(const FString& Parameters)
{
	UWorld::InitializationValues Values;
	Values.AllowAudioPlayback(false).CreatePhysicsScene(true).CreateNavigation(false).CreateAISystem(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true,
		ERHIFeatureLevel::Num, &Values);
	if (!TestNotNull(TEXT("Test world"), World)) return false;
	AActor* Owner = World->SpawnActor<AActor>();
	USceneComponent* Root = NewObject<USceneComponent>(Owner);
	Owner->SetRootComponent(Root);
	Root->RegisterComponent();
	Owner->SetActorLocation(FVector(0, 0, 100));
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>(Owner);
	Owner->AddInstanceComponent(Inventory);
	Inventory->RegisterComponent();
	UDataTable* Table = NewObject<UDataTable>(Inventory);
	Table->RowStruct = FItemRow::StaticStruct();
	FindFProperty<FObjectPropertyBase>(Inventory->GetClass(), TEXT("ItemDefinitions"))
		->SetObjectPropertyValue_InContainer(Inventory, Table);
	FIntProperty* ShieldCapacity = FindFProperty<FIntProperty>(Inventory->GetClass(), TEXT("ShieldsSlotCapacity"));
	ShieldCapacity->SetPropertyValue_InContainer(Inventory, 2);
	FItemRow Shield;
	Shield.Category = EItemCategory::Shields;
	Shield.GuardPower = 60;
	Shield.PowerVariance = 3;
	Shield.MaxStackSize = 3;
	Shield.WorldMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	Table->AddRow(TEXT("Shield"), Shield);
	Table->AddRow(TEXT("OtherShield"), Shield);

	TestTrue(TEXT("Add first shield stack"), Inventory->TryAddItemWithPower(TEXT("Shield"), 2, 60));
	if (Inventory->GetEntries().IsEmpty()) { World->DestroyWorld(false); return false; }
	const FGuid First = Inventory->GetEntries()[0].EntryId;
	TestTrue(TEXT("Same row and power merge"), Inventory->TryAddItemWithPower(TEXT("Shield"), 1, 60));
	TestEqual(TEXT("Three matching shields use one slot"), Inventory->GetEntries().Num(), 1);
	TestEqual(TEXT("Quantity is three"), Inventory->FindEntry(First)->Quantity, 3);
	TestTrue(TEXT("Different power gets another slot"), Inventory->TryAddItemWithPower(TEXT("Shield"), 1, 61));
	TestEqual(TEXT("Two power variants occupy two slots"), Inventory->GetEntries().Num(), 2);
	TestFalse(TEXT("Full matching stack cannot overflow capacity"), Inventory->TryAddItemWithPower(TEXT("Shield"), 1, 60));
	TestEqual(TEXT("Failed addition preserves first stack"), Inventory->FindEntry(First)->Quantity, 3);
	TestTrue(TEXT("Matching partial stack accepts items at slot capacity"), Inventory->TryAddItemWithPower(TEXT("Shield"), 2, 61));
	TestEqual(TEXT("Power is independent from quantity"), Inventory->FindEntry(First)->EquipmentPower, 60);
	TestFalse(TEXT("Different item type cannot merge despite same power"), Inventory->TryAddItemWithPower(TEXT("OtherShield"), 1, 60));
	ShieldCapacity->SetPropertyValue_InContainer(Inventory, 4);
	TestTrue(TEXT("Different type uses its own slot"), Inventory->TryAddItemWithPower(TEXT("OtherShield"), 1, 60));
	TestEqual(TEXT("Third slot created"), Inventory->GetEntries().Num(), 3);
	Shield.MaxOwnedQuantity = 6;
	Table->AddRow(TEXT("Shield"), Shield);
	TestFalse(TEXT("Ownership limit counts every power variant"), Inventory->TryAddItemWithPower(TEXT("Shield"), 1, 62));
	TestFalse(TEXT("Negative power rejected"), Inventory->TryAddItemWithPower(TEXT("Shield"), 1, -1));

	FItemRow Sword = Shield;
	Sword.Category = EItemCategory::Weapon;
	Sword.AttackPower = 26;
	Sword.MaxOwnedQuantity = 0;
	Table->AddRow(TEXT("Sword"), Sword);
	TestTrue(TEXT("Add sword"), Inventory->TryAddItemWithPower(TEXT("Sword"), 1, 26));
	TestTrue(TEXT("Equal sword rolls merge"), Inventory->TryAddItemWithPower(TEXT("Sword"), 2, 26));
	TestEqual(TEXT("Sword quantity"), Inventory->GetEntries().Last().Quantity, 3);
	TestTrue(TEXT("Different sword power separates"), Inventory->TryAddItemWithPower(TEXT("Sword"), 1, 27));
	TestEqual(TEXT("New sword retains its power"), Inventory->GetEntries().Last().EquipmentPower, 27);
	for (int32 Index = 0; Index < 100; ++Index)
	{
		const int32 Roll = Sword.RollEquipmentPower();
		TestTrue(TEXT("Sword roll uses attack base and variance"), Roll >= 23 && Roll <= 29);
	}
	FItemRow Food;
	Food.Category = EItemCategory::Food;
	Food.MaxStackSize = 999;
	Table->AddRow(TEXT("Food"), Food);
	TestTrue(TEXT("Food addition"), Inventory->TryAddItemWithPower(TEXT("Food"), 2, 60));
	TestTrue(TEXT("Food ignores equipment power"), Inventory->TryAddItemWithPower(TEXT("Food"), 1, 61));
	TestEqual(TEXT("Food still stacks normally"), Inventory->GetEntries().Last().Quantity, 3);
	TestEqual(TEXT("Food power normalized"), Inventory->GetEntries().Last().EquipmentPower, 0);

	FItemRow RandomShield = Shield;
	RandomShield.MaxOwnedQuantity = 0;
	RandomShield.MaxStackSize = 999;
	Table->AddRow(TEXT("RandomShield"), RandomShield);
	ShieldCapacity->SetPropertyValue_InContainer(Inventory, 3);
	AItemPickup* Pickup = World->SpawnActorDeferred<AItemPickup>(AItemPickup::StaticClass(),
		FTransform(FVector(200, 0, 100)), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	Pickup->InitializeDroppedItem(Table, TEXT("RandomShield"), INDEX_NONE);
	Pickup->FinishSpawning(FTransform(FVector(200, 0, 100)));
	TestFalse(TEXT("No capacity rejects pickup"), Pickup->TryInteract(Owner));
	const int32 RolledPower = Pickup->GetEquipmentPower();
	TestTrue(TEXT("Shield roll uses guard base and variance"), RolledPower >= 57 && RolledPower <= 63);
	TestFalse(TEXT("Retry still fails when full"), Pickup->TryInteract(Owner));
	TestEqual(TEXT("Failed pickup never rerolls"), Pickup->GetEquipmentPower(), RolledPower);
	ShieldCapacity->SetPropertyValue_InContainer(Inventory, 4);
	TestTrue(TEXT("Pickup succeeds after freeing capacity"), Pickup->TryInteract(Owner));
	const FGuid RandomId = Inventory->GetEntries().Last().EntryId;
	TestEqual(TEXT("Collected entry retains world roll"), Inventory->FindEntry(RandomId)->EquipmentPower, RolledPower);
	TestTrue(TEXT("Create three matching copies"), Inventory->TryAddItemWithPower(TEXT("RandomShield"), 2, RolledPower));

	AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>();
	Floor->GetStaticMeshComponent()->SetStaticMesh(Shield.WorldMesh);
	Floor->SetActorScale3D(FVector(20, 20, 1));
	Floor->SetActorLocation(FVector(0, 0, -50));
	Floor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
	FText Failure;
	TestTrue(TEXT("Drop equipment copy"), Inventory->TryDropItem(RandomId, Failure));
	TestEqual(TEXT("Drop removes only one"), Inventory->FindEntry(RandomId)->Quantity, 2);
	AItemPickup* Dropped = nullptr;
	for (TActorIterator<AItemPickup> It(World); It; ++It)
		if (!It->IsActorBeingDestroyed()) Dropped = *It;
	if (TestNotNull(TEXT("Dropped equipment pickup"), Dropped))
	{
		TestEqual(TEXT("Dropped pickup retains original power"), Dropped->GetEquipmentPower(), RolledPower);
		TestTrue(TEXT("Recollect equipment"), Dropped->TryInteract(Owner));
		TestFalse(TEXT("No double collection"), Dropped->TryInteract(Owner));
		TestEqual(TEXT("Recollection merges back into original stack"), Inventory->FindEntry(RandomId)->Quantity, 3);
		TestEqual(TEXT("Round trip never rerolls power"), Inventory->FindEntry(RandomId)->EquipmentPower, RolledPower);
	}
	TestTrue(TEXT("Delete one equipment copy"), Inventory->TryDeleteItem(RandomId, Failure));
	TestEqual(TEXT("Delete leaves the remaining copies"), Inventory->FindEntry(RandomId)->Quantity, 2);
	TestEqual(TEXT("Delete preserves power"), Inventory->FindEntry(RandomId)->EquipmentPower, RolledPower);
	World->DestroyWorld(false);
	return true;
}
#endif
