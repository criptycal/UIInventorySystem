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
class UWrapBox;
class UItemSlotWidget;


UCLASS()
class UIINVENTORYSYSTEM_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	virtual FReply NativeOnPreviewKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;

	virtual FReply NativeOnKeyUp(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;

	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

private:
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


	// Close button support is kept here for later reuse; Tab handles closing.
	// UPROPERTY(meta = (BindWidget))
	// TObjectPtr<UButton> Button_0;

	// UFUNCTION()
	// void HandleCloseButtonClicked();
};
