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
- Apply damage and healing through the HealthComponent, with immediate heart updates.
- Show the heart bar during gameplay and switch to the inventory's heart bar while it is open.
- Test damage and healing with temporary inventory buttons, each changing health by half a heart.

The current UI contains the inventory heading, currency display, background blur, and heart bar. Item slots, drag-and-drop, equipment, and other complete inventory features are still future work.

## How C++ and the editor work together

A native class defines behavior and exposes selected properties to Unreal. A Blueprint child supplies the visual assets and configured defaults that behavior uses. Compiling a C++ class makes it available to Unreal; the relevant Blueprint must also inherit from it.

| C++ class | Editor connection | Responsibility |
| --- | --- | --- |
| `AUIInventorySystemCharacter` | `BP_ThirdPersonCharacter` | Owns the currency and health components; forwards the inventory input request to the controller. |
| `AUIInventorySystemPlayerController` | `BP_ThirdPersonPlayerController` defaults | Registers input mappings; owns the inventory and gameplay HUD; switches visibility, input mode, and cursor state. |
| `UCurrencyComponent` | Inherited component on the character | Owns the balance and validates additions/spending. |
| `UHealthComponent` | Inherited component on the character | Owns maximum/current health, validates damage/healing, initializes full health, and broadcasts changes. |
| `AMoneyPickup` | `BP_Money` and its placed instances | Creates collision/mesh components, reads a currency row, and handles collection. |
| `FMoneyRow` | `DT_MoneyNative` | Defines each row's display name, amount, and mesh. |
| `UInventoryWidget` | Parent of `WBP_Inventory` | Handles UI keyboard input, updates currency text, and forwards test-button clicks to the HealthComponent. |
| `UHealthBarWidget` | Parent of `WBP_HealthBar` | Creates heart widgets, subscribes to health changes, updates their fill, and removes its subscription during destruction. |
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

The inventory also uses `BindWidgetOptional` for `DamageTestButton` and `HealTestButton`, both Buttons. Their names must match when present, but they can be removed from the Designer without making those bindings mandatory. Their click handlers are registered once in `NativeOnInitialized()`. Set **Is Focusable = false** on these test buttons in the Designer so keyboard focus stays with the inventory.

### Editor properties and class selections

- `EditDefaultsOnly` exposes shared defaults, such as maximum health, the heart textures, and the widget class to create.
- `EditAnywhere` also allows per-instance configuration, such as a placed pickup's `CurrencyRow` or a heart's `Percent`.
- `VisibleInstanceOnly` exposes runtime values for inspection, such as current health and currency balance, without allowing direct Details-panel editing.
- `BlueprintPure` and `BlueprintCallable` expose C++ functions to Blueprint graphs if needed. The current native implementation can call them directly.
- `TSubclassOf<UHeartWidget>` stores a **class to instantiate**, while `TObjectPtr<UHeartWidget>` would refer to an existing object.

Set the controller's **Inventory Widget Class** to `WBP_Inventory`, and `WBP_HealthBar`'s **Heart Widget Class** to `WBP_Heart`. These Blueprint classes include the Designer layouts and texture assignments needed by the native behavior.

Set the controller's **Player HUD Widget Class** to `WBP_PlayerHUD`. This asset inherits from `UserWidget` and contains a `WBP_HealthBar`; it needs no Blueprint graph logic or separate native HUD class.

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

A maximum of `13.5` creates 14 slots, with the last slot half full even at maximum health. Use a whole-number maximum when every container should be full at maximum health.

### Damage, healing, and notifications

[HealthComponent.cpp](Source/UIInventorySystem/Private/Inventory/Components/HealthComponent.cpp) provides `ApplyDamage(float)` and `Heal(float)`. Both reject nonfinite or nonpositive amounts. Damage stops at zero; healing stops at maximum health. They return `true` only when current health changes, and `false` for rejected requests or no change.

After a successful change, the component broadcasts `OnHealthChanged(CurrentHealth, MaxHealth)`. It also broadcasts after initializing full health in `BeginPlay()`, allowing a bar constructed before component initialization to receive that initial state. Health remains owned by the component; neither the inventory nor the HUD stores a separate health total. Healing from zero is currently allowed; death and revival rules are not implemented yet.

The temporary inventory buttons call `ApplyDamage(0.5f)` and `Heal(0.5f)` on the owning pawn's HealthComponent:

```text
Inventory button click
  -> HealthComponent validates and changes health
  -> OnHealthChanged broadcasts
  -> Subscribed health bars update each heart's texture
```

### Building the bar

[HealthBarWidget.cpp](Source/UIInventorySystem/Private/UI/HealthBarWidget.cpp) runs this process in `NativeConstruct()`:

1. Remove any previous health subscription and clear `HealthBarWrapBox`, preventing duplicate bindings and children when reconstruction occurs.
2. Find the owning player's pawn and its HealthComponent.
3. Calculate `HeartCount = FMath::CeilToInt(MaxHealth)`.
4. Create a `WBP_Heart` for each slot and set its percentage.
5. Add each created heart to the Wrap Box.
6. Subscribe to the component's `OnHealthChanged` delegate with `AddUniqueDynamic`.

The per-heart calculation is:

```cpp
const float HeartPercent = FMath::Clamp(
    CurrentHealth - static_cast<float>(Index), 0.0f, 1.0f);
```

For `CurrentHealth = 2.5`, indices `0`, `1`, and `2` produce `1.0`, `1.0`, and `0.5`. The maximum determines slot count; current health determines fill. A fractional maximum is rounded up to provide a slot for its partial heart.

`HandleHealthChanged()` applies that same calculation to the existing children, so damage and healing update the display without polling or rebuilding the bar. `NativeDestruct()` removes the subscription and clears the component reference. Slot count is established during construction; changing maximum health dynamically and rebuilding the slot count is not implemented yet.

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

## Gameplay HUD and inventory visibility

The local player controller subscribes to `OnPossessedPawnChanged` in `BeginPlay()` and also processes its current pawn immediately. `HandleHUDPawnChanged()` removes the previous HUD, then creates the configured `WBP_PlayerHUD` with the controller as its owning player when a pawn is available. This gives its embedded health bar a way to find the pawn's HealthComponent.

The HUD is added to the player screen at Z-order zero. During gameplay it uses `HitTestInvisible` (**Not Hit-Testable: Self & All Children**) so its visuals do not intercept pointer input. Opening the inventory collapses the gameplay HUD; closing the inventory restores it. The collapsed HUD remains subscribed to health changes, so it shows the latest health when gameplay resumes.

In `WBP_PlayerHUD`, the Canvas Panel supplies the viewport layout and a `WBP_HealthBar` child supplies the existing native heart behavior. Configure its anchors and position in Designer; **Size to Content** on its Canvas slot lets the health bar use its desired size. The gameplay and inventory bars are separate widget instances reading the same component.

The controller recreates the HUD when possession changes, but complete respawn behavior, inventory rebinding across pawn replacement, and health persistence still need their own implementation and tests.

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
- Controller: **Inventory Widget Class = WBP_Inventory**, **Player HUD Widget Class = WBP_PlayerHUD**.
- `WBP_Inventory`: parent **InventoryWidget**, with `MoneyText` and a `WBP_HealthBar` child.
- Optional inventory test buttons: `DamageTestButton` and `HealTestButton`, with **Is Focusable** disabled.
- `WBP_PlayerHUD`: parent **UserWidget**, with a Canvas Panel and `WBP_HealthBar` child; configure the child's Canvas slot sizing/anchors and keep the HUD non-hit-testable.
- `WBP_HealthBar`: parent **HealthBarWidget**, with `HealthBarWrapBox` and **Heart Widget Class = WBP_Heart**.
- `WBP_Heart`: parent **HeartWidget**, with `HeartImage` and all three texture references assigned.
- Pickups: select valid rows from `DT_MoneyNative` through **Currency Row**.

## Verification and remaining work

The November 4, 2026 Development Editor build succeeded. Manual Play-in-Editor checks have covered Tab opening/closing, currency collection, generated heart counts, full/half/empty textures, damage/healing buttons, immediate health updates, and the gameplay HUD displaying the same health after closing the inventory. Normal initialization is `CurrentHealth = MaxHealth`; the buttons now provide a way to test partial health without changing initialization. There is no automated CI test suite configured yet.

To repeat the health check, start Play, confirm the gameplay hearts appear, open the inventory with Tab, and click the damage/heal buttons. Each click should change half a heart, stop at zero or maximum, and update the visible bar immediately. Close the inventory and confirm that the gameplay HUD shows the same health. Repeat opening and closing to check that bars and notifications are not duplicated.

Still to implement:

- Gameplay damage sources (such as an enemy), healing items, and death/revival rules.
- Runtime maximum-health changes and corresponding heart-slot rebuilding.
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
