// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Data/ItemCategory.h"
#include "InventoryWidget.generated.h"

/**
 *
 */

class UButton;
class UTextBlock;
class UImage;
class UTexture2D;
struct FItemRow;
class UWrapBox;
class UItemSlotWidget;
class UBorder;
class UScrollBox;
class UHorizontalBox;
class UInventoryComponent;
struct FInventoryEntry;
class UItemActionMenuWidget;
enum class EInventoryItemAction : uint8;
class AInventoryCharacterPreview;
class UMaterialInstanceDynamic;
class UAnimSequence;
class UEquipmentComponent;
class UStaticMesh;


UCLASS()
class UIINVENTORYSYSTEM_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual FReply NativeOnPreviewKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;

	virtual FReply NativeOnKeyUp(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;

	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseMove(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonUp(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnMouseCaptureLost(
		const FCaptureLostEvent& CaptureLostEvent) override;




private:
	UPROPERTY(Transient)
	TObjectPtr<UItemActionMenuWidget> ActionMenu;
	TMap<TWeakObjectPtr<UItemSlotWidget>, FGuid> VisibleEntryIds;
	TWeakObjectPtr<UInventoryComponent> ObservedInventory;
	FGuid ActionEntryId;
	void OpenActionMenu();
	void CloseActionMenu();
	void HandleMenuDismissed(bool bCloseInventory);
	void HandleItemAction(EInventoryItemAction Action);
	void HandleInventoryChanged();

	bool bCloseOnTabRelease = false;

	bool bCloseOnBRelease = false;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MoneyText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> DamageTestButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> HealTestButton;

	UFUNCTION()
	void HandleDamageTestButtonClicked();

	UFUNCTION()
	void HandleHealTestButtonClicked();

	EItemCategory CurrentCategory = EItemCategory::Weapon;

	// Each category keeps its chosen display order for this widget's lifetime.
	TSet<EItemCategory> SortedCategories;
	void SortCurrentCategory();
	void SortCategoryEntries(TArray<FInventoryEntry>& Entries, const UInventoryComponent& Inventory) const;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CategoryTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> WeaponsFilterButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ShieldsFilterButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FoodFilterButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WeaponsFilterIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ShieldsFilterIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> FoodFilterIcon;

	UFUNCTION()
	void HandleWeaponsFilterClicked();

	UFUNCTION()
	void HandleShieldsFilterClicked();

	UFUNCTION()
	void HandleFoodFilterClicked();

	void SelectCategory(EItemCategory NewCategory);

	UFUNCTION()
	void UpdateCategoryVisuals();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> ItemGrid;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UItemSlotWidget> ItemSlotClass;

	void RebuildItemGrid();

	// These refer to visible slots, not item types: two identical swords remain distinct selections.
	TWeakObjectPtr<UItemSlotWidget> SelectedSlot;
	TWeakObjectPtr<UItemSlotWidget> HoveredSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> ItemInfoPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemQuantityText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> ItemStatRow;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemStatValueText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemDescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ItemDescriptionScroll;

	void HandleItemClicked(UItemSlotWidget* SlotWidget);
	void HandleItemHoverChanged(UItemSlotWidget* SlotWidget, bool bHovered);
	void ClearItemSelection();
	void RefreshItemInfo();
	void RefreshItemStat(const FItemRow& Definition, const FInventoryEntry& Entry);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemStatIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Appearance")
	TObjectPtr<UTexture2D> WeaponStatIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Appearance")
	TObjectPtr<UTexture2D> ShieldStatIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Appearance")
	TObjectPtr<UTexture2D> FoodStatIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PreviousPageButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NextPageButton;

	static constexpr int32 ItemsPerPage = 20;

	int32 CurrentPageIndex = 0;
	int32 TotalPages = 0;

	UFUNCTION()
	void HandlePreviousPageClicked();

	UFUNCTION()
	void HandleNextPageClicked();

	void UpdatePageButtons();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CharacterPreviewImage;

	UPROPERTY(Transient)
	TObjectPtr<AInventoryCharacterPreview> CharacterPreviewActor;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CharacterPreviewMaterial;

	void CreateCharacterPreview();
	void ReleaseCharacterPreview();

	TWeakObjectPtr<UEquipmentComponent> ObservedEquipment;

	void RefreshPreviewEquipment();
	UStaticMesh* FindPreviewEquipmentMesh(const FGuid& EntryId) const;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Preview")
	TObjectPtr<UAnimSequence> CharacterPreviewIdleAnimation;

	// Applied whenever the inventory creates its preview actor.
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Preview", meta = (ClampMin = "50.0", Units = "cm"))
	float PreviewCameraDistance = 280.f;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Preview", meta = (Units = "cm"))
	float PreviewCameraHeight = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Preview", meta = (ClampMin = "0.0", ToolTip = "Key light intensity in lumens."))
	float PreviewKeyLightIntensity = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Preview", meta = (ClampMin = "0.0", ToolTip = "Fill light intensity in lumens."))
	float PreviewFillLightIntensity = 150.f;

	bool bDraggingCharacterPreview = false;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Preview",
		meta = (ClampMin = "0.0", ToolTip = "Degrees to rotate per horizontal UI unit dragged"))
	float PreviewRotationSensitivity = 0.5f;



	// Close button support is kept here for later reuse; Tab handles closing.
	// UPROPERTY(meta = (BindWidget))
	// TObjectPtr<UButton> Button_0;

	// UFUNCTION()
	// void HandleCloseButtonClicked();
};
