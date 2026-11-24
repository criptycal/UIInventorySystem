// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "ItemSlotWidget.generated.h"


class UImage;
class UTextBlock;
class UTexture2D;
class UBorder;
class UItemSlotWidget;
struct FItemRow;

DECLARE_DELEGATE_OneParam(FItemSlotClicked, UItemSlotWidget*);
DECLARE_DELEGATE_OneParam(FItemSlotContextMenuRequested, UItemSlotWidget*);
DECLARE_DELEGATE_TwoParams(FItemSlotHoverChanged, UItemSlotWidget*, bool);
/**
 *
 */
UCLASS()
class UIINVENTORYSYSTEM_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetItemDisplay(FName InRowName, const FItemRow& Definition, int32 InQuantity);
	void SetEmptyDisplay();
	void SetSelected(bool bInSelected);
	void SetEquipped(bool bInEquipped);
	FName GetItemRowName() const { return DisplayRowName; }
	int32 GetQuantity() const { return DisplayQuantity; }

	FItemSlotClicked OnItemClicked;
	FItemSlotContextMenuRequested OnContextMenuRequested;
	FItemSlotHoverChanged OnHoverChanged;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> QuantityText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> SlotBackground;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> QuantityBadge;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Appearance")
	FLinearColor HoverOutlineColor = FLinearColor(0.95f, 0.95f, 0.85f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Appearance")
	FLinearColor SelectionColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|Appearance")
	FLinearColor EquippedBackgroundColor = FLinearColor(0.02f, 0.35f, 0.85f, 1.0f);

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> DisplayIcon = nullptr;

	int32 DisplayQuantity = 0;
	FName DisplayRowName = NAME_None;
	bool bItemHovered = false;
	bool bSelected = false;
	bool bEquipped = false;
	FSlateBrush DefaultSlotBrush;

	void RefreshDisplay();
	void RefreshVisuals();
};
