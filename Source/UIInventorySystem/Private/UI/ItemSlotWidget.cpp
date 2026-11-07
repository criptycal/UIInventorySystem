// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UItemSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    RefreshDisplay();
}

void UItemSlotWidget::SetItemDisplay(
    UTexture2D* InIcon,
    int32 InQuantity)
{
    DisplayIcon = InIcon;
    DisplayQuantity = FMath::Max(0, InQuantity);

    RefreshDisplay();
}

void UItemSlotWidget::RefreshDisplay()
{
    if (ItemIcon)
    {
        ItemIcon->SetBrushFromTexture(DisplayIcon.Get(), false);

        const bool bShowIcon =
            DisplayIcon != nullptr && DisplayQuantity > 0;

        ItemIcon->SetVisibility(
            bShowIcon
            ? ESlateVisibility::HitTestInvisible
            : ESlateVisibility::Hidden);
    }

    if (QuantityText)
    {
        const bool bShowQuantity = DisplayQuantity > 1;

        QuantityText->SetText(
            bShowQuantity
            ? FText::AsNumber(DisplayQuantity)
            : FText::GetEmpty());

        QuantityText->SetVisibility(
            bShowQuantity
            ? ESlateVisibility::HitTestInvisible
            : ESlateVisibility::Collapsed);
    }
}