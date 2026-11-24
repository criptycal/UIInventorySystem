// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemSlotWidget.h"
#include "Inventory/Data/ItemRow.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Components/Border.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Rendering/DrawElementTypes.h"
#include "Rendering/SlateResourceHandle.h"

namespace
{
    UTexture2D* GetEquippedSlotGradient()
    {
        // The border brushes keep this shared transient texture alive while in use.
        static TWeakObjectPtr<UTexture2D> CachedGradient;
        if (CachedGradient.IsValid()) return CachedGradient.Get();

        constexpr int32 Resolution = 64;
        UTexture2D* Texture = UTexture2D::CreateTransient(Resolution, Resolution, PF_B8G8R8A8);
        if (!Texture) return nullptr;

        Texture->SRGB = true;
        Texture->Filter = TF_Bilinear;
        Texture->AddressX = TA_Clamp;
        Texture->AddressY = TA_Clamp;
        FColor* Pixels = static_cast<FColor*>(Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE));
        for (int32 Y = 0; Y < Resolution; ++Y)
        {
            for (int32 X = 0; X < Resolution; ++X)
            {
                const float U = 2.f * X / (Resolution - 1) - 1.f;
                const float V = 2.f * Y / (Resolution - 1) - 1.f;
                const float Falloff = FMath::Clamp(1.f - (U * U + V * V) * 0.65f, 0.f, 1.f);
                const float Brightness = FMath::Lerp(0.12f, 1.f, Falloff * Falloff);
                Pixels[Y * Resolution + X] = FLinearColor(Brightness, Brightness, Brightness, 1.f).ToFColor(true);
            }
        }
        Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
        Texture->UpdateResource();
        CachedGradient = Texture;
        return Texture;
    }
}

void UItemSlotWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    if (SlotBackground)
    {
        DefaultSlotBrush = SlotBackground->Background;
    }
}

void UItemSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetVisibility(ESlateVisibility::Visible);
    RefreshDisplay();
    RefreshVisuals();
}

void UItemSlotWidget::SetItemDisplay(
    FName InRowName,
    const FItemRow& Definition,
    int32 InQuantity)
{
    DisplayRowName = InRowName;
    DisplayIcon = Definition.Icon;
    DisplayQuantity = FMath::Max(0, InQuantity);

    RefreshDisplay();
}

void UItemSlotWidget::SetEmptyDisplay()
{
    DisplayRowName = NAME_None;
    DisplayIcon = nullptr;
    DisplayQuantity = 0;
    bItemHovered = false;
    bSelected = false;
    bEquipped = false;
    OnItemClicked.Unbind();
    OnContextMenuRequested.Unbind();
    OnHoverChanged.Unbind();
    RefreshDisplay();
    RefreshVisuals();
}

void UItemSlotWidget::RefreshDisplay()
{
    // Empty slots retain their frame but cannot intercept hover or selection.
    SetVisibility(DisplayQuantity > 0 ? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible);
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

    // Every occupied slot displays its quantity; power belongs in the details panel.
    const bool bShowBadge = DisplayQuantity > 0;
    if (QuantityText)
    {
        QuantityText->SetText(
            bShowBadge
            ? FText::AsNumber(DisplayQuantity)
            : FText::GetEmpty());

        QuantityText->SetVisibility(
            bShowBadge
            ? ESlateVisibility::HitTestInvisible
            : ESlateVisibility::Collapsed);
    }

    if (QuantityBadge)
    {
        QuantityBadge->SetVisibility(bShowBadge
            ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
}

void UItemSlotWidget::SetSelected(bool bInSelected)
{
    bInSelected = bInSelected && DisplayQuantity > 0;
    if (bSelected == bInSelected) return;
    bSelected = bInSelected;
    RefreshVisuals();
}

void UItemSlotWidget::SetEquipped(bool bInEquipped)
{
    bInEquipped = bInEquipped && DisplayQuantity > 0;
    if (bEquipped == bInEquipped) return;
    bEquipped = bInEquipped;
    RefreshVisuals();
}

void UItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
    if (DisplayQuantity <= 0) return;
    bItemHovered = true;
    RefreshVisuals();
    OnHoverChanged.ExecuteIfBound(this, true);
}

void UItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    bItemHovered = false;
    RefreshVisuals();
    OnHoverChanged.ExecuteIfBound(this, false);
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (DisplayQuantity <= 0) return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	const FKey Button = InMouseEvent.GetEffectingButton();

	if (Button == EKeys::LeftMouseButton)
	{
		OnItemClicked.ExecuteIfBound(this);
		return FReply::Handled();
	}

	if (Button == EKeys::RightMouseButton)
	{
		OnContextMenuRequested.ExecuteIfBound(this);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UItemSlotWidget::RefreshVisuals()
{
    if (SlotBackground)
    {
        FSlateBrush Brush = DefaultSlotBrush;
        if (bEquipped)
        {
            Brush.TintColor = EquippedBackgroundColor;
            Brush.SetResourceObject(GetEquippedSlotGradient());
        }
        if (bSelected)
        {
            // NativePaint supplies one fine glowing frame instead of doubling the border.
            Brush.OutlineSettings.Width = 0.f;
        }
        else if (bItemHovered)
        {
            Brush.OutlineSettings.Color = HoverOutlineColor;
            Brush.OutlineSettings.Width = 1.0f;
        }
        SlotBackground->SetBrush(Brush);
    }
    InvalidateLayoutAndVolatility();
}

int32 UItemSlotWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    const int32 TopLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect,
        OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
    if (!bSelected) return TopLayer;

    // Four filled arrowheads point outward toward the selected slot's corners.
    const FVector2f Size(AllottedGeometry.GetLocalSize());
    const float Inset = 4.0f;
    if (Size.X <= Inset * 2.f || Size.Y <= Inset * 2.f) return TopLayer;

    const float Length = FMath::Min(8.0f, (FMath::Min(Size.X, Size.Y) - Inset * 2.f) * 0.08f);
    const FLinearColor SelectionTint = SelectionColor * InWidgetStyle.GetColorAndOpacityTint();
    const FColor Tint = SelectionTint.ToFColor(true);

    const float FrameInset = Inset + 1.5f;
    if (Size.X <= FrameInset * 2.f || Size.Y <= FrameInset * 2.f) return TopLayer;
    const TArray<FVector2D> FramePoints = {
        FVector2D(FrameInset, FrameInset), FVector2D(Size.X - FrameInset, FrameInset),
        FVector2D(Size.X - FrameInset, Size.Y - FrameInset), FVector2D(FrameInset, Size.Y - FrameInset),
        FVector2D(FrameInset, FrameInset)
    };
    // Broad, faint strokes soften the glow around the 1.75-unit bright outline.
    const float StrokeWidths[] = { 8.f, 5.f, 3.f, 1.75f };
    const float StrokeOpacities[] = { 0.04f, 0.10f, 0.22f, 0.95f };
    for (int32 Stroke = 0; Stroke < UE_ARRAY_COUNT(StrokeWidths); ++Stroke)
    {
        FLinearColor StrokeColor = SelectionTint;
        StrokeColor.A *= StrokeOpacities[Stroke];
        FSlateDrawElement::MakeLines(OutDrawElements, TopLayer + 1, AllottedGeometry.ToPaintGeometry(),
            FramePoints, ESlateDrawEffect::None, StrokeColor, true, StrokeWidths[Stroke]);
    }
    const FSlateRenderTransform& PaintTransform = AllottedGeometry.GetAccumulatedRenderTransform();
    TArray<FSlateVertex> Vertices;
    TArray<SlateIndex> Indices;
    Vertices.Reserve(12);
    Indices.Reserve(12);

    for (int32 Corner = 0; Corner < 4; ++Corner)
    {
        const bool bRight = (Corner & 1) != 0;
        const bool bBottom = (Corner & 2) != 0;
        const FVector2f Point(bRight ? Size.X - Inset : Inset, bBottom ? Size.Y - Inset : Inset);
        const float DirectionX = bRight ? -1.f : 1.f;
        const float DirectionY = bBottom ? -1.f : 1.f;
        const FVector2f Points[] = {
            Point,
            Point + FVector2f(DirectionX * Length, 0.f),
            Point + FVector2f(0.f, DirectionY * Length)
        };
        const SlateIndex FirstVertex = static_cast<SlateIndex>(Vertices.Num());
        for (const FVector2f& Position : Points)
        {
            Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(
                PaintTransform, Position, FVector2f::ZeroVector, Tint));
        }
        // Mirrored corners retain the same triangle winding.
        Indices.Add(FirstVertex);
        Indices.Add(FirstVertex + (bRight != bBottom ? 2 : 1));
        Indices.Add(FirstVertex + (bRight != bBottom ? 1 : 2));
    }

    // An empty resource handle draws solid vertex colors without an image asset.
    FSlateDrawElement::MakeCustomVerts(OutDrawElements, TopLayer + 2,
        FSlateResourceHandle(), Vertices, Indices, nullptr, 0, 0);
    return TopLayer + 2;
}
