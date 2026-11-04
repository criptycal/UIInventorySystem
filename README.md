# UIInventorySystem

An Unreal Engine 5 learning project focused on implementing inventory UI behavior in **C++**, with Unreal Editor and UMG Designer used for layouts, textures, meshes, and configuration.

The project began with Unreal's Third Person template while following *Unreal Engine 5 UI Design Advance Inventory System with UE5*. The inventory, currency pickups, and heart display have progressively moved into native C++ classes. This README describes the current implementation and how its code connects to the editor assets.

## What works so far

- Open and close the inventory with **Tab**, including keyboard focus, cursor visibility, and input-mode switching.
- Collect currency through native pickup overlap handling.
- Configure pickup denominations and meshes using a data table backed by a C++ row struct.
- Store currency on the character and display its balance when the inventory widget is constructed.
- Store maximum/current health on the character in **heart units**.
- Generate the heart widgets automatically from maximum health.
- Display full, half, and empty heart textures.

The current UI contains the inventory heading, currency display, background blur, and heart bar. Item slots, drag-and-drop, equipment, and other complete inventory features are still future work.

## How C++ and the editor work together

A native class defines behavior and exposes selected properties to Unreal. A Blueprint child supplies the visual assets and configured defaults that behavior uses. Compiling a C++ class makes it available to Unreal; the relevant Blueprint must also inherit from it.

| C++ class | Editor connection | Responsibility |
| --- | --- | --- |
| `AUIInventorySystemCharacter` | `BP_ThirdPersonCharacter` | Owns the currency and health components; forwards the inventory input request to the controller. |
| `AUIInventorySystemPlayerController` | Player-controller Blueprint defaults | Registers input mappings; creates/reuses the configured inventory widget; switches input mode and cursor state. |
| `UCurrencyComponent` | Inherited component on the character | Owns the balance and validates additions/spending. |
| `UHealthComponent` | Inherited component on the character | Owns maximum/current health and initializes the character at full health. |
| `AMoneyPickup` | `BP_Money` and its placed instances | Creates collision/mesh components, reads a currency row, and handles collection. |
| `FMoneyRow` | `DT_MoneyNative` | Defines each row's display name, amount, and mesh. |
| `UInventoryWidget` | Parent of `WBP_Inventory` | Handles UI keyboard input and updates the currency text. |
| `UHealthBarWidget` | Parent of `WBP_HealthBar` | Reads the character's health and creates the required heart widgets. |
| `UHeartWidget` | Parent of `WBP_Heart` | Chooses the texture for one heart. |

### Native components

The [character constructor](Source/UIInventorySystem/UIInventorySystemCharacter.cpp) creates its gameplay components:

```cpp
CurrencyComponent = CreateDefaultSubobject<UCurrencyComponent>(TEXT("CurrencyComponent"));
HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
```

That is why they appear as inherited components in `BP_ThirdPersonCharacter`. They already belong to the character; adding another copy through the editor is unnecessary. Each component owns its state, while the UI reads that state.

### Binding Designer widgets to C++

`BindWidget` connects a C++ property to a widget in the Blueprint's Designer hierarchy. The name and type must match:

| Widget Blueprint | Required Designer name | Type |
| --- | --- | --- |
| `WBP_Inventory` | `MoneyText` | Text Block |
| `WBP_HealthBar` | `HealthBarWrapBox` | Wrap Box |
| `WBP_Heart` | `HeartImage` | Image |

For example, this declaration in [HeartWidget.h](Source/UIInventorySystem/Public/UI/HeartWidget.h) refers to the existing Designer image:

```cpp
UPROPERTY(meta = (BindWidget))
TObjectPtr<UImage> HeartImage;
```

It does not create an image. `WBP_Heart` provides the image and layout; C++ uses the bound reference to change its brush texture. Keep **Is Variable** enabled for these named widgets.

### Editor properties and class selections

- `EditDefaultsOnly` exposes shared defaults, such as maximum health, the heart textures, and the widget class to create.
- `EditAnywhere` also allows per-instance configuration, such as a placed pickup's `CurrencyRow` or a heart's `Percent`.
- `VisibleInstanceOnly` exposes runtime values for inspection, such as current health and currency balance, without allowing direct Details-panel editing.
- `BlueprintPure` and `BlueprintCallable` expose C++ functions to Blueprint graphs if needed. The current native implementation can call them directly.
- `TSubclassOf<UHeartWidget>` stores a **class to instantiate**, while `TObjectPtr<UHeartWidget>` would refer to an existing object.

Set the controller's **Inventory Widget Class** to `WBP_Inventory`, and `WBP_HealthBar`'s **Heart Widget Class** to `WBP_Heart`. These Blueprint classes include the Designer layouts and texture assignments needed by the native behavior.

Saved Blueprint defaults can override C++ defaults. Changing `MaxHealth` in the header does not guarantee that an existing configured character Blueprint will use that new value; inspect its inherited HealthComponent in the editor.

## Inventory input and currency display

The input path is:

```text
IA_Inventory / Tab
  -> Character::OpenInventorySystem()
  -> PlayerController::ToggleInventory()
  -> ShowInventory() or HideInventory()
```

[The controller](Source/UIInventorySystem/UIInventorySystemPlayerController.cpp) creates the inventory widget with an owning player and reuses it. Opening adds it to the player screen, shows the cursor, suppresses look input, and focuses the widget using UI-only input. Closing removes it and restores game-only input and look control.

Once UI-only input is active, [InventoryWidget.cpp](Source/UIInventorySystem/Private/UI/InventoryWidget.cpp) handles Tab itself. A fresh key-down arms closing; key-up performs the close. This avoids returning control to gameplay while the closing Tab key is still held. The initial opening release does not close the inventory because it has not armed that flag.

`NativeConstruct()` reads `GetOwningPlayerPawn()`, finds its CurrencyComponent, and writes the balance to `MoneyText` with `FText::AsNumber`. Currency changes are currently refreshed through widget construction; the text is not yet subscribed to the component's change event. The former close-button handler is retained as commented code; Tab is the active close control.

## Currency pickups and data tables

[CurrencyComponent.cpp](Source/UIInventorySystem/Private/Inventory/Components/CurrencyComponent.cpp) owns an integer balance. `TryAddCurrency()` rejects nonpositive amounts and integer overflow. `TrySpendCurrency()` rejects nonpositive amounts and insufficient funds. Successful changes broadcast `OnBalanceChanged`.

[MoneyRow.h](Source/UIInventorySystem/Public/Inventory/Data/MoneyRow.h) defines the native data-table row:

```text
DisplayName : FText
Amount      : int32
Mesh        : UStaticMesh reference
```

In the editor, `DT_MoneyNative` supplies the rows, and each `BP_Money` instance selects a **Currency Row** containing both a table and row name. The configured denominations are Green = 1, Blue = 5, Gold = 10, Red = 15, and Silver = 3.

[MoneyPickup.cpp](Source/UIInventorySystem/Private/Inventory/Pickups/MoneyPickup.cpp) uses that data in two places:

1. **Construction:** reset the cached amount/mesh, validate the table's row type and selected row, then apply its amount and mesh. This also supports visual preview while editing pickups.
2. **Overlap:** find a CurrencyComponent on the overlapping actor, guard against duplicate collection, and attempt the addition. Only a successful addition disables collision and destroys the pickup. A failed addition leaves it available.

The sphere collision and static-mesh components are created in C++. The editor supplies the row selection and assets. Legacy `DT_Money` / `S_Money` assets remain in the project; the native pickup implementation uses `FMoneyRow` and `DT_MoneyNative`.

## Health is measured directly in hearts

[HealthComponent.h](Source/UIInventorySystem/Public/Inventory/Components/HealthComponent.h) stores `float MaxHealth` and `float CurrentHealth`. The native maximum defaults to `3.0f`; the character Blueprint may configure a different maximum. At `BeginPlay()`, maximum health is constrained to at least `1.0f` and current health is set to that maximum.

| Maximum | Current | Display |
| --- | --- | --- |
| `3.0` | `3.0` | Three full hearts |
| `3.0` | `2.5` | Two full hearts and one half |
| `3.0` | `2.0` | Two full hearts and one empty |
| `3.0` | `0.0` | Three empty hearts |

The project previously used two integer health points per heart. It now uses heart units throughout, so there is no division by two.

### Building the bar

[HealthBarWidget.cpp](Source/UIInventorySystem/Private/UI/HealthBarWidget.cpp) runs this process in `NativeConstruct()`:

1. Clear `HealthBarWrapBox`, preventing duplicate children when reconstruction occurs.
2. Find the owning player's pawn and its HealthComponent.
3. Calculate `HeartCount = FMath::CeilToInt(MaxHealth)`.
4. Create a `WBP_Heart` for each slot and set its percentage.
5. Add each created heart to the Wrap Box.

The per-heart calculation is:

```cpp
const float HeartPercent = FMath::Clamp(
    CurrentHealth - static_cast<float>(Index), 0.0f, 1.0f);
```

For `CurrentHealth = 2.5`, indices `0`, `1`, and `2` produce `1.0`, `1.0`, and `0.5`. The maximum determines slot count; current health determines fill. A fractional maximum is rounded up to provide a slot for its partial heart.

### Choosing the texture

[HeartWidget.cpp](Source/UIInventorySystem/Private/UI/HeartWidget.cpp) clamps incoming values in `SetPercent()` and refreshes the image. `NativePreConstruct()` also refreshes it for Designer previews and runtime construction.

The current texture thresholds are:

- `Percent >= 1.0`: full.
- `0.5 <= Percent < 1.0`: half.
- `Percent < 0.5`: empty.

This is a three-state visual display, not a continuous fill. Arbitrary fractions are shown using those thresholds.

Assign these properties in **WBP_Heart -> Class Defaults -> Heart**:

| Property | Texture in `Content/Inventory/Misc` |
| --- | --- |
| Full Heart Texture | `ICO_heartFull` |
| Half Heart Texture | `ICO_heartHalf` |
| Empty Heart Texture | `ICO_heartEmpty` |

The code replaces the image's brush texture. Assigning a texture only to the Designer image does not populate these properties; leaving them empty can produce a white square at runtime.

### Layout and widget lifecycle

Heart size, padding, anchors, and available row width belong to the Designer layout. For example, 30-unit hearts fit 15 across a 450-unit container when there is no additional horizontal spacing. Wrap Box layout handles the next row automatically.

The heart bar creates its children during runtime `NativeConstruct()`, so its Designer container can be empty. An individual `WBP_Heart` can preview its configured percentage through `NativePreConstruct()`.

`NativeConstruct()` can run more than once as a widget's underlying UI is rebuilt. It is not a one-time gameplay initialization hook. `BeginPlay()` initializes the gameplay component; the widgets read that state when constructed.

## Open and run the project

The project is associated with **Unreal Engine 5.8** and has been built locally with **5.8.3**, Windows, Visual Studio C++ tools, and a Windows SDK. The checked-in `.vsconfig` records the local Visual Studio component selections. The engine itself is not included.

Install Git LFS before cloning so Unreal receives the actual assets rather than small pointer files:

```sh
git lfs install
git clone https://github.com/criptycal/UIInventorySystem.git
cd UIInventorySystem
git lfs pull
```

1. Install the matching Unreal version and its required C++ build tools. The enabled engine plugins are listed in `UIInventorySystem.uproject`.
2. Generate Visual Studio project files from `UIInventorySystem.uproject`.
3. Build `UIInventorySystemEditor` with **Development Editor / Win64**, then open the `.uproject`.
4. Open `/Game/ThirdPerson/Lvl_ThirdPerson` if it is not already loaded, press Play, collect currency, and press Tab to open the inventory.

For changes to reflected property types, native inheritance, or default subobjects, close the editor, perform a normal build, and reopen it. This is the workflow used for the component and widget migrations in this project.

### Editor configuration checklist

- Character: inherited CurrencyComponent and HealthComponent are present; check the configured **Max Health**.
- Input: `IA_Inventory` is assigned to the character's inventory action, and the controller registers the mapping context containing Tab.
- Controller: **Inventory Widget Class = WBP_Inventory**.
- `WBP_Inventory`: parent **InventoryWidget**, with `MoneyText` and a `WBP_HealthBar` child.
- `WBP_HealthBar`: parent **HealthBarWidget**, with `HealthBarWrapBox` and **Heart Widget Class = WBP_Heart**.
- `WBP_Heart`: parent **HeartWidget**, with `HeartImage` and all three texture references assigned.
- Pickups: select valid rows from `DT_MoneyNative` through **Currency Row**.

## Verification and remaining work

Development Editor builds have succeeded. Manual Play-in-Editor checks have covered Tab opening/closing, currency collection, generated heart counts, and full/half/empty textures. The partial-heart check temporarily started current health at `2.5f`; normal initialization has been restored to `CurrentHealth = MaxHealth`. There is no automated CI test suite configured yet.

Still to implement:

- Damage and healing operations, death behavior, and health-change notifications.
- Updating the displayed hearts immediately when health changes while the UI stays open.
- Subscribing the currency text to the existing `OnBalanceChanged` delegate.
- Item storage, slots, stacking, equipment, and other inventory interactions.
- Save/load, persistence across character replacement, and multiplayer replication.

Health and currency currently belong to character components and are not persistent saved player data.

## Repository contents

```text
Source/UIInventorySystem/
  Public/Inventory/Components/   CurrencyComponent and HealthComponent headers
  Private/Inventory/Components/  Component implementations
  Public/Inventory/Data/         FMoneyRow
  Public/Inventory/Pickups/      MoneyPickup header
  Private/Inventory/Pickups/     Pickup implementation
  Public/UI/                    InventoryWidget, HealthBarWidget, HeartWidget
  Private/UI/                   Widget implementations
  UIInventorySystemCharacter.*
  UIInventorySystemPlayerController.*
Content/Inventory/              Widgets, data tables, meshes, textures, and related assets
Content/ThirdPerson/             Character Blueprints and demonstration level
Config/                         Project configuration
Docs/                           Earlier learning notes and UI guide
```

The template's combat, platforming, and side-scrolling examples are also retained. Notes in `Docs/` record earlier stages and can describe superseded plans; this README summarizes the current implementation.

Unreal `.uasset` and `.umap` files use **Git LFS**. Source, content, configuration, and the `.uproject` are tracked. Generated folders (`Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`), generated solution files, and local IDE/assistant settings are excluded and recreated locally as needed.
