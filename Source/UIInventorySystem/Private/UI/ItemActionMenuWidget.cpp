#include "UI/ItemActionMenuWidget.h"

#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Rendering/DrawElementTypes.h"

void UItemActionMenuWidget::Configure(const FText& InItemName, bool bInFood)
{
	ItemName = InItemName;
	bFood = bInFood;
	ActiveOption = 0;
	Status = NSLOCTEXT("Inventory", "ActionHint", "Choose an action for one item.");
}

namespace
{
	class SActionSelectionBorder : public SBorder
	{
	public:
		SLATE_BEGIN_ARGS(SActionSelectionBorder)
			: _Selected(false)
			{}
			SLATE_ATTRIBUTE(bool, Selected)
			SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			Selected = InArgs._Selected;

			SBorder::Construct(
				SBorder::FArguments()
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor_Lambda([this]
					{
						return Selected.Get()
							? FLinearColor(1.f, 1.f, 0.9f, 1.f)
							: FLinearColor(0.35f, 0.35f, 0.3f, 0.7f);
					})
				.Padding(1.5f)
				[
					InArgs._Content.Widget
				]
			);
		}

		virtual int32 OnPaint(
			const FPaintArgs& Args,
			const FGeometry& Geometry,
			const FSlateRect& CullingRect,
			FSlateWindowElementList& DrawElements,
			int32 LayerId,
			const FWidgetStyle& WidgetStyle,
			bool bParentEnabled) const override
		{
			const int32 TopLayer = SBorder::OnPaint(
				Args, Geometry, CullingRect, DrawElements,
				LayerId, WidgetStyle, bParentEnabled
			);

			if (!Selected.Get()) return TopLayer;

			const FVector2D Size = Geometry.GetLocalSize();
			const double Inset = 2.0;
			const double Length = 10.0;
			const FLinearColor Tint =
				FLinearColor(1.f, 1.f, 0.9f) * WidgetStyle.GetColorAndOpacityTint();

			for (int32 Corner = 0; Corner < 4; ++Corner)
			{
				const bool bRight = Corner % 2 != 0;
				const bool bBottom = Corner >= 2;

				const FVector2D Point(
					bRight ? Size.X - Inset : Inset,
					bBottom ? Size.Y - Inset : Inset);

				const TArray<FVector2D> Points = {
					Point + FVector2D(bRight ? -Length : Length, 0.0),
					Point,
					Point + FVector2D(0.0, bBottom ? -Length : Length)
				};

				const auto DrawStroke = [&](float Thickness, float Opacity)
					{
						FLinearColor StrokeColor = Tint;
						StrokeColor.A *= Opacity;

						FSlateDrawElement::MakeLines(
							DrawElements,
							TopLayer + 1,
							Geometry.ToPaintGeometry(),
							Points,
							ESlateDrawEffect::None,
							StrokeColor,
							true,
							Thickness
						);
					};

				DrawStroke(9.f, 0.08f); // Wide, faint outer glow.
				DrawStroke(6.f, 0.18f); // Brighter inner glow.
				DrawStroke(3.f, 1.f);  // Sharp corner marker.
			}
			return TopLayer + 1;
		}

	private:
		TAttribute<bool> Selected;
	};
}

void UItemActionMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	SetKeyboardFocus();
}

TSharedRef<SWidget> UItemActionMenuWidget::RebuildWidget()
{
	SetIsFocusable(true);
	return SNew(SOverlay)
	+ SOverlay::Slot()
	[
		SNew(SButton).ButtonStyle(FCoreStyle::Get(), "NoBorder")
		.OnClicked_UObject(this, &UItemActionMenuWidget::Cancel)
		.ContentPadding(0.f)
		[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, 0.38f))]
	]
	+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
	[
		SNew(SBox).WidthOverride(380.f)
		[
			SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.75f, 0.78f, 0.8f)).Padding(1.5f)
			[
				SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(FLinearColor(0.025f, 0.035f, 0.045f, 1.f)).Padding(20.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 14)
					[SNew(STextBlock).Text(ItemName).AutoWrapText(true)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22)).ColorAndOpacity(FLinearColor::White)]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
					[SNew(SBox).HeightOverride(1.f)[SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(1, 1, 1, 0.5f))]]
					+ SVerticalBox::Slot().AutoHeight()[MakeOption(0, bFood
						? NSLOCTEXT("Inventory", "EatOne", "Eat")
						: NSLOCTEXT("Inventory", "EquipItem", "Equip"))]
					+ SVerticalBox::Slot().AutoHeight()[MakeOption(1, NSLOCTEXT("Inventory", "DropOne", "Drop"))]
					+ SVerticalBox::Slot().AutoHeight()[MakeOption(2, NSLOCTEXT("Inventory", "DeleteOne", "Delete"))]
					+ SVerticalBox::Slot().AutoHeight()[MakeOption(3, NSLOCTEXT("Inventory", "CancelAction", "Cancel"))]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 14, 0, 0)
					[SNew(STextBlock).Text_Lambda([this] { return Status; }).AutoWrapText(true)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
					.ColorAndOpacity(FLinearColor(0.8f, 0.85f, 0.9f))]
				]
			]
		]
	];
}

TSharedRef<SWidget> UItemActionMenuWidget::MakeOption(
	int32 Index, const FText& Label)
{
	return SNew(SBox)
		.HeightOverride(64.f)
		.Padding(FMargin(0.f, 4.f))
		[
			SNew(SButton)
				.ButtonStyle(FCoreStyle::Get(), "NoBorder")
				.IsFocusable(false)
				.ContentPadding(0.f)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.OnHovered_Lambda([this, Index]
					{
						ActiveOption = Index;
					})
				.OnClicked_Lambda([this, Index]
					{
						return ActivateOption(Index);
					})
				[
					SNew(SActionSelectionBorder)
						.Selected_Lambda([this, Index]
							{
								return ActiveOption == Index;
							})
						[
							SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(
									FLinearColor(0.008f, 0.008f, 0.008f, 1.f))
								.Padding(FMargin(12.f, 5.f))
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
										.Text_Lambda([this, Index, Label]
											{
												return Index == 2 && bConfirmDelete
													? NSLOCTEXT(
														"Inventory",
														"ConfirmDeleteOne",
														"Confirm delete (1 item)")
													: Label;
											})
										.Font(FCoreStyle::GetDefaultFontStyle("BoldItalic", 22))
										.ColorAndOpacity(FLinearColor::White)
								]
						]
				]
		];
}


void UItemActionMenuWidget::ShowFailure(const FText& Message)
{
	Status = Message;
	bConfirmDelete = false;
}

FReply UItemActionMenuWidget::ActivateOption(int32 Index)
{
	if (Index < 0 || Index > 3) return FReply::Handled();
	if (Index == 3) return Cancel();

	if (Index == 2 && !bConfirmDelete)
	{
		bConfirmDelete = true;
		Status = NSLOCTEXT(
			"Inventory", "DeleteWarning",
			"Permanently delete one item? Select Confirm delete, or Cancel to keep it.");
		return FReply::Handled();
	}

	bConfirmDelete = false;

	EInventoryItemAction Action;
	switch (Index)
	{
	case 0:
		Action = bFood
			? EInventoryItemAction::Eat
			: EInventoryItemAction::Equip;
		break;

	case 1:
		Action = EInventoryItemAction::Drop;
		break;

	default:
		Action = EInventoryItemAction::Delete;
		break;
	}

	OnActionRequested.ExecuteIfBound(Action);
	return FReply::Handled();
}

FReply UItemActionMenuWidget::Cancel()
{
	OnDismissed.ExecuteIfBound(false);
	return FReply::Handled();
}

FReply UItemActionMenuWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	const FKey Key = Event.GetKey();
	if (Key == EKeys::Up || Key == EKeys::Down)
	{
		const int32 First = 0;
		const int32 Count = 4 - First;
		ActiveOption = First + (ActiveOption - First + (Key == EKeys::Down ? 1 : Count - 1)) % Count;
		return FReply::Handled();
	}
	if (!Event.IsRepeat())
	{
		if (Key == EKeys::Enter || Key == EKeys::A || Key == EKeys::SpaceBar) return ActivateOption(ActiveOption);
		if (Key == EKeys::Escape || Key == EKeys::B) return Cancel();
		if (Key == EKeys::Tab)
		{
			OnDismissed.ExecuteIfBound(true);
			return FReply::Handled();
		}
	}
	return FReply::Handled(); // Modal: never send gameplay/inventory shortcuts through it.
}
