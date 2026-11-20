#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Inventory/Components/InventoryComponent.h"
#include "Inventory/Components/HealthComponent.h"
#include "Inventory/Data/ItemRow.h"
#include "Pickups/ItemPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryActionTest,
	"UIInventorySystem.Inventory.ItemActions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryActionTest::RunTest(const FString& Parameters)
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
	UHealthComponent* Health = NewObject<UHealthComponent>(Owner);
	Owner->AddInstanceComponent(Health);
	Health->RegisterComponent();
	Health->Heal(Health->GetMaxHealth());

	UDataTable* Table = NewObject<UDataTable>(Inventory);
	Table->RowStruct = FItemRow::StaticStruct();
	FItemRow Food;
	Food.Category = EItemCategory::Food;
	Food.MaxStackSize = 2;
	Food.HealthRestored = 1.5f;
	Food.WorldMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	Table->AddRow(TEXT("TestFood"), Food);
	FItemRow Weapon;
	Weapon.WorldMesh = Food.WorldMesh;
	Table->AddRow(TEXT("TestWeapon"), Weapon);
	// Test fixture supplies a transient table without modifying the game's assets.
	FindFProperty<FObjectPropertyBase>(Inventory->GetClass(), TEXT("ItemDefinitions"))
		->SetObjectPropertyValue_InContainer(Inventory, Table);
	FText Failure;
	Inventory->TryAddItem(TEXT("TestFood"), 3);
	const FGuid First = Inventory->GetEntries()[0].EntryId;
	const FGuid Second = Inventory->GetEntries()[1].EntryId;
	TestTrue(TEXT("Same item stacks have distinct IDs"), First != Second);
	TestFalse(TEXT("Eating at full health fails"), Inventory->TryEatItem(First, Failure));
	TestEqual(TEXT("Full health preserves quantity"), Inventory->FindEntry(First)->Quantity, 2);
	Health->ApplyDamage(0.5f);
	TestTrue(TEXT("Eating heals"), Inventory->TryEatItem(First, Failure));
	TestEqual(TEXT("Healing clamps to max"), Health->GetCurrentHealth(), Health->GetMaxHealth());
	TestEqual(TEXT("Eating consumes exactly one"), Inventory->FindEntry(First)->Quantity, 1);
	TestTrue(TEXT("Delete selected second stack"), Inventory->TryDeleteItem(Second, Failure));
	TestNull(TEXT("Empty stack removed"), Inventory->FindEntry(Second));
	TestNotNull(TEXT("Other identical stack preserved"), Inventory->FindEntry(First));
	TestFalse(TEXT("Stale ID cannot delete another stack"), Inventory->TryDeleteItem(Second, Failure));
	Inventory->TryAddItem(TEXT("TestWeapon"), 2);
	const FGuid Sword = Inventory->GetEntries().Last().EntryId;
	Health->ApplyDamage(1.f);
	TestFalse(TEXT("Weapons cannot be eaten"), Inventory->TryEatItem(Sword, Failure));
	TestFalse(TEXT("Drop fails with no ground"), Inventory->TryDropItem(First, Failure));
	TestNotNull(TEXT("Failed drop retains item"), Inventory->FindEntry(First));

	AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>();
	Floor->GetStaticMeshComponent()->SetStaticMesh(Food.WorldMesh);
	Floor->SetActorScale3D(FVector(20, 20, 1));
	Floor->SetActorLocation(FVector(0, 0, -50));
	Floor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
	TestTrue(TEXT("Drop succeeds on clear floor"), Inventory->TryDropItem(First, Failure));
	TestNull(TEXT("Dropped item removed from inventory"), Inventory->FindEntry(First));
	AItemPickup* Dropped = nullptr;
	int32 PickupCount = 0;
	for (TActorIterator<AItemPickup> It(World); It; ++It) { Dropped = *It; ++PickupCount; }
	TestEqual(TEXT("Exactly one world pickup"), PickupCount, 1);
	if (Dropped)
	{
		TestTrue(TEXT("Dropped item can be collected again"), Dropped->TryInteract(Owner));
		TestFalse(TEXT("Cannot collect it twice"), Dropped->TryInteract(Owner));
		int32 FoodCount = 0;
		for (const FInventoryEntry& Entry : Inventory->GetEntries())
			if (Entry.ItemRowName == TEXT("TestFood")) FoodCount += Entry.Quantity;
		TestEqual(TEXT("Drop/collect round trip preserves one item"), FoodCount, 1);
	}
	World->DestroyWorld(false);
	return true;
}
#endif
