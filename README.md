# UIInventorySystem

An Unreal Engine 5 learning project focused on implementing inventory UI behavior in **C++**, with Unreal Editor and UMG Designer used for layouts, textures, meshes, and configuration.

The project began with Unreal's Third Person template while following *Unreal Engine 5 UI Design Advance Inventory System with UE5*. The inventory, currency pickups, and heart display have progressively moved into native C++ classes. This README describes the current implementation and how its code connects to the editor assets.

## What works so far

- Open and close the inventory with **Tab**, including keyboard focus, cursor visibility, and input-mode switching.
- Close the open inventory with keyboard **B**, using the same controller cleanup as Tab.
- Collect currency through native pickup overlap handling.
- Focus nearby item pickups and press **E** to collect them, with a contextual HUD prompt and white outline.
- Store item stacks, display their icons and quantities, and filter the inventory grid by category.
- Rotate and float both item and currency meshes through the shared pickup base.
- Configure pickup denominations and meshes using a data table backed by a C++ row struct.
- Store currency on the character and display its balance when the inventory widget is constructed.
- Store maximum/current health on the character in **heart units**.
- Generate the heart widgets automatically from maximum health.
- Display full, half, and empty heart textures.
- Apply damage and healing through the HealthComponent, with immediate heart updates.
- Show the heart bar during gameplay and switch to the inventory's heart bar while it is open.
- Select Weapons, Shields, or Food categories, with a matching title, selection highlight, and mouse-hover feedback.
- Retain temporary damage/healing buttons, each changing health by half a heart; their parent is currently collapsed to hide them.

The current UI contains the inventory heading, currency display, background blur, heart bar, item grid, a shared footer, and Adventure Log/System navigation prompts. Only Back has implemented behavior among these prompts; Sort, Select, Rotate, Adventure Log, and System are visual placeholders. Item selection, drag-and-drop, equipment, and other complete inventory features are still future work.

## How C++ and the editor work together

A native class defines behavior and exposes selected properties to Unreal. A Blueprint child supplies the visual assets and configured defaults that behavior uses. Compiling a C++ class makes it available to Unreal; the relevant Blueprint must also inherit from it.

| C++ class | Editor connection | Responsibility |
| --- | --- | --- |
| `AUIInventorySystemCharacter` | `BP_ThirdPersonCharacter` | Owns currency, health, inventory, and interaction components; forwards input and adds the outline material to its camera. |
| `AUIInventorySystemPlayerController` | `BP_ThirdPersonPlayerController` defaults | Registers input mappings; owns the inventory and gameplay HUD; switches visibility, input mode, and cursor state. |
| `UCurrencyComponent` | Inherited component on the character | Owns the balance and validates additions/spending. |
| `UHealthComponent` | Inherited component on the character | Owns maximum/current health, validates damage/healing, initializes full health, and broadcasts changes. |
| `AMoneyPickup` | `BP_Money` and its placed instances | Creates collision/mesh components, reads a currency row, and handles collection. |
| `FMoneyRow` | `DT_MoneyNative` | Defines each row's display name, amount, and mesh. |
| `UInventoryComponent` | Inherited component on the character | Stores entries, validates item definitions, fills existing stacks, and creates additional stacks. |
| `APickupBase` | Abstract parent of item and money pickups | Shares components, rotation/bobbing, collection guards, and successful-pickup destruction. |
| `AItemPickup` | `BP_ItemPickup` | Reads an item row, supplies its mesh, collects through interaction, and toggles mesh highlighting. |
| `FItemRow` / `FInventoryEntry` | Item definitions / runtime entries | Separate shared item data from each stack's row name and quantity. |
| `UInteractionComponent` | Inherited component on the character | Finds a nearby visible item, broadcasts focus changes, and rechecks focus when interacting. |
| `UInteractionPromptWidget` | Parent of `WBP_InteractionPrompt`, embedded in `WBP_PlayerHUD` | Shows or hides the pickup prompt in response to focus changes. |
| `UItemSlotWidget` | Parent of `WBP_ItemSlot` | Displays the item icon and a quantity when greater than one. |
| `UInventoryWidget` | Parent of `WBP_Inventory` | Handles UI keyboard input, currency text, category selection and hover visuals, and health test-button clicks. |
| `UHealthBarWidget` | Parent of `WBP_HealthBar` | Creates heart widgets, subscribes to health changes, updates their fill, and removes its subscription during destruction. |
| `UHeartWidget` | Parent of `WBP_Heart` | Chooses the texture for one heart. |

### Native components

The [character constructor](Source/UIInventorySystem/UIInventorySystemCharacter.cpp) creates its gameplay components:

```cpp
CurrencyComponent = CreateDefaultSubobject<UCurrencyComponent>(TEXT("CurrencyComponent"));
HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("InteractionComponent"));
```

That is why they appear as inherited components in `BP_ThirdPersonCharacter`. They already belong to the character; adding another copy through the editor is unnecessary. Each component owns its state, while the UI reads that state.

### Binding Designer widgets to C++

`BindWidget` connects a C++ property to a widget in the Blueprint's Designer hierarchy. The name and type must match:

| Widget Blueprint | Required Designer name | Type |
| --- | --- | --- |
| `WBP_Inventory` | `MoneyText` | Text Block |
| `WBP_Inventory` | `CategoryTitle` | Text Block |
| `WBP_Inventory` | `WeaponsFilterButton`, `ShieldsFilterButton`, `FoodFilterButton` | Button |
| `WBP_Inventory` | `WeaponsFilterIcon`, `ShieldsFilterIcon`, `FoodFilterIcon` | Image |
| `WBP_HealthBar` | `HealthBarWrapBox` | Wrap Box |
| `WBP_Heart` | `HeartImage` | Image |
| `WBP_Inventory` | `ItemGrid` | Wrap Box |
| `WBP_ItemSlot` | `ItemIcon`, `QuantityText` | Image, Text Block |
| `WBP_InteractionPrompt` | `PromptContainer` | Border |

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

Once UI-only input is active, [InventoryWidget.cpp](Source/UIInventorySystem/Private/UI/InventoryWidget.cpp) handles **Tab and B** through `NativeOnPreviewKeyDown()` and `NativeOnKeyUp()`. A fresh key-down arms that key's closing flag; releasing the matching key calls the controller's `HideInventory()`. Repeated key-down events are consumed without arming a new close request. Both flags reset in `NativeConstruct()` and before closing. The initial opening Tab release does not close the inventory because it has not armed its flag. B only closes an open inventory; it does not open it during gameplay.

Enhanced Input handles the gameplay inventory action. The focused widget receives these UI keyboard events directly, so B does not require another Input Action or Mapping Context. These UI keys are currently hardcoded as `EKeys::Tab` and `EKeys::B`; the controller-style icons do not imply gamepad support or configurable key bindings.

`NativeConstruct()` reads `GetOwningPlayerPawn()`, finds its CurrencyComponent, and writes the balance to `MoneyText` with `FText::AsNumber`. Currency changes are currently refreshed through widget construction; the text is not yet subscribed to the component's change event. The former close-button handler is retained as commented code; Tab and B are the active close controls.

### Footer and header prompts

`WBP_Inventory` supplies the visual prompt hierarchy in UMG Designer. These are groups of Text Blocks and Images, not clickable Buttons, and require no native `BindWidget` properties or Blueprint graph logic.

```text
Canvas Panel
  FooterPrompts (Horizontal Box)
    SortPrompt    -> Sort + Y icon
    BackPrompt    -> Back + B icon
    SelectPrompt  -> Select + A icon
    RotatePrompt  -> Rotate + R icon
  AdventureLogSizeBox
    AdventureLogVerticalBox -> L icon above Adventure Log
  SystemGuideSizeBox
    Vertical Box -> R icon above System
```

The footer is anchored bottom-right, aligned `(1, 1)`, positioned at `(-60, -30)`, and uses **Size to Content**. Prompt slots use Auto sizing and vertical centering. Each icon has 8 units of left padding; each group except Rotate has 32 units of right padding. The footer uses **Not Hit-Testable (Self & All Children)** so its hints do not intercept pointer input.

Adventure Log and System use top-center anchors and alignment `(0.5, 0.5)`, with positions `(-250, 125)` and `(250, 125)`. Adventure Log has a 200-unit width to accommodate its longer label, a 32-by-32 L icon, and the same italic font styling as System. The currency box uses a top-right anchor, alignment `(1, 0)`, and position `(-60, 56)` to separate it from System. These offsets establish the current layout; narrow windows still need visual checks for overlap.

The controller-style icons currently follow the visual reference. Sort and Select still need their actions and item selection behavior, Rotate needs a character preview, and the header prompts need destination screens before their actions can be implemented in C++.

### Category selection and hover feedback

[ItemCategory.h](Source/UIInventorySystem/Public/Inventory/Data/ItemCategory.h) defines `EItemCategory` with `Weapon`, `Shields`, and `Food`. These are the three categories currently included in this project.

`NativeOnInitialized()` binds the category buttons' click, hover, and unhover events. Clicking calls `SelectCategory()`, which validates the category, stores `CurrentCategory`, updates the localized title, and calls `UpdateCategoryVisuals()`. Hover events call only `UpdateCategoryVisuals()`, so moving the mouse does not change the selected category.

The appearance function uses a local lambda to apply the same rules to each icon: selected opacity is `1.0`, hovered but unselected opacity is `0.7`, and idle opacity is `0.4`. Selection takes priority over hover. These updates are event-driven and do not require Tick. `NativeConstruct()` refreshes the current selection when the cached inventory widget is reopened.

The Designer supplies this layout:

```text
CategorySection (Vertical Box)
  CategoryTitle
  CategoryButtons (Horizontal Box)
    Size Box -> WeaponsFilterButton -> WeaponsFilterIcon
    Size Box -> ShieldsFilterButton -> ShieldsFilterIcon
    Size Box -> FoodFilterButton -> FoodFilterIcon
  CategoryDividerSizeBox -> CategoryDivider (Border)
```

Buttons are non-focusable and their background brushes use No Draw Type, leaving the icons as their visible content. The divider uses a 600-by-1 Size Box, an Auto slot with 8 units of top padding, and a white Border at alpha `0.25` with zero content padding. The divider group is not hit-testable. The item grid is rebuilt when opening the inventory or changing category, using matching entries from the character's InventoryComponent.

The health test buttons remain inside `HorizontalBox_108`, whose visibility is **Collapsed**. To use them again, set that parent to **Not Hit-Testable (Self Only)**, compile and save the widget, and start a fresh Play session. Their existing C++ handlers remain available.

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

## Item interaction and outline

`FItemRow` holds the display name, description, category, icon, world mesh, maximum stack size, and sort priority. `UInventoryComponent::TryAddItem()` fills compatible stacks before splitting the remaining quantity into new entries. Items with maximum stack size one occupy separate slots. The pickup and inventory must reference the same definitions table.

`APickupBase::TryInteract()` guards against duplicate collection, calls the subclass's `TryCollect()`, and disables collision and destroys the actor only after success. `AMoneyPickup` opts into overlap collection. `AItemPickup` instead blocks the Visibility trace on its sphere and waits for explicit interaction. Both keep the shared mesh rotation and sine-wave bobbing.

The item input and focus flow is:

```text
IA_PickupInteract / E (Started)
  -> Character::Interact()
  -> InteractionComponent::Interact()
  -> recheck focus -> ItemPickup::TryInteract()
  -> InventoryComponent::TryAddItem()
```

The interaction component traces from the camera each frame, checks the candidate's distance from the character, and checks for an obstruction from the character's eyes. Native defaults are a 1000-unit camera trace and 300-unit interaction range. `SetFocusedItem()` disables the previous item's highlight, enables the new one, and broadcasts only when the target changes. It also clears focus after collection and during `EndPlay()`.

`WBP_InteractionPrompt` supplies the E/Pickup layout. Its native parent subscribes to `OnFocusedItemChanged`, synchronizes the initial state, and removes its subscription in `NativeDestruct()`. Its required `PromptContainer` is a Border. `WBP_PlayerHUD` contains this prompt alongside the health bar; the controller collapses the gameplay HUD while the inventory is open.

`M_ItemOutline` is a Post Process material at Scene Color After Tonemapping. The project enables Custom Depth with Stencil (`r.CustomDepth=3`). Item meshes use stencil ID 1 and write Custom Depth only while focused. The material compares four neighboring stencil samples to form an inner border, then compares CustomDepth against SceneDepth with a small tolerance to suppress covered pixels. Its Custom expression is HLSL shader code; C++ controls focus and mesh flags.

In `BP_ThirdPersonCharacter` Class Defaults, assign `IA_PickupInteract` to **Interact Action** and `M_ItemOutline` to **Item Outline Material**. The character's native `BeginPlay()` adds the assigned material to `FollowCamera`. No Blueprint graph logic or post-process volume is required. A missing material produces a warning.

Performance follow-up: the camera currently keeps the outline pass active even without a focused item. Disabling that pass when idle and comparing GPU timings at fixed resolution are planned work; no performance measurements have been established yet.

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
- `WBP_Inventory`: parent **InventoryWidget**, with `MoneyText`, a `WBP_HealthBar` child, and the required category bindings listed above.
- Optional inventory test buttons: `DamageTestButton` and `HealTestButton`, with **Is Focusable** disabled.
- `WBP_PlayerHUD`: parent **UserWidget**, with a Canvas Panel and `WBP_HealthBar` child; configure the child's Canvas slot sizing/anchors and keep the HUD non-hit-testable.
- `WBP_HealthBar`: parent **HealthBarWidget**, with `HealthBarWrapBox` and **Heart Widget Class = WBP_Heart**.
- `WBP_Heart`: parent **HeartWidget**, with `HeartImage` and all three texture references assigned.
- Pickups: select valid rows from `DT_MoneyNative` through **Currency Row**.

## Verification and remaining work

On November 8, 2026, the latest recorded Development Editor build succeeded at 14:13 local time, after the camera-material changes. The user confirmed manual pickup, automatic currency pickup, and the focus-driven prompt during Play. Later runtime logs record successful food and weapon pickups. The outline graph, stencil configuration, and character material assignment were inspected through the editor tools; these checks do not substitute for visual occlusion testing or GPU profiling.

The November 4, 2026 Development Editor build succeeded. Manual Play-in-Editor checks have covered Tab opening/closing, currency collection, generated heart counts, full/half/empty textures, damage/healing buttons, immediate health updates, and the gameplay HUD displaying the same health after closing the inventory. Normal initialization is `CurrentHealth = MaxHealth`; the buttons now provide a way to test partial health without changing initialization. There is no automated CI test suite configured yet.

Keyboard B closing and the Back prompt were also confirmed working in Play. The footer hierarchy, spacing, textures, and visibility were inspected in the editor. On November 5, the Adventure Log prompt was added, `WBP_Inventory` compiled and saved successfully, and its placement was checked in Designer. This was a widget compilation, not a new full C++ build or a complete viewport-size test.

The category code has been reviewed, and the divider hierarchy, padding, dimensions, tint, and visibility have been inspected in the editor. This review does not establish a new full C++ build or automated runtime test result. To verify the category behavior in Play, hover each unselected icon, move away, select another category, and reopen the inventory. Check that the title follows clicks, selected icons stay bright, and hover alone never changes selection.

To repeat the health check, first restore the test-button parent's visibility as described above. Start Play, confirm the gameplay hearts appear, open the inventory with Tab, and click the damage/heal buttons. Each click should change half a heart, stop at zero or maximum, and update the visible bar immediately. Close the inventory and confirm that the gameplay HUD shows the same health. Repeat opening and closing to check that bars and notifications are not duplicated.

Still to implement:

- Gameplay damage sources (such as an enemy), healing items, and death/revival rules.
- Runtime maximum-health changes and corresponding heart-slot rebuilding.
- Subscribing the currency text to the existing `OnBalanceChanged` delegate.
- Item selection, equipment, consumption, removal, inventory capacity, and live grid-change notifications.
- Disable the outline post-process pass when idle and profile its GPU cost.
- Sort/Select actions, a rotatable character preview, and Adventure Log/System screens with C++ navigation.
- Configurable UI key bindings and gamepad input, with prompts matching the active controls.
- Save/load, persistence across character replacement, and multiplayer replication.

Health, currency, and inventory currently belong to character components and are not persistent saved player data.

## Repository contents

```text
Source/UIInventorySystem/
  Public/Inventory/Components/   CurrencyComponent, HealthComponent, InventoryComponent
  Private/Inventory/Components/  Component implementations
  Public/Inventory/Data/         Money/item definitions, inventory entries, categories
  Public/Inventory/Pickups/      MoneyPickup header
  Private/Inventory/Pickups/     Pickup implementation
  Public/Pickups/                PickupBase and ItemPickup headers
  Private/Pickups/               Shared and item-specific pickup behavior
  Public/Interaction/Components/ InteractionComponent header
  Private/Interaction/Components/ Focus detection and interaction
  Public/UI/                    Inventory, item slot, health, and interaction prompt widgets
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
