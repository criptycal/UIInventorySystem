#include "UI/ItemActionMenuWidget.h"

#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

void UItemActionMenuWidget::Configure(const FText& InItemName, bool bInFood)
{
	ItemName = InItemName;
	bFood = bInFood;
	ActiveOption = 0;
	Status = NSLOCTEXT("Inventory", "ActionHint", "Choose an action for one item.");
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
		SNew(SBox).WidthOverride(340.f)
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

TSharedRef<SWidget> UItemActionMenuWidget::MakeOption(int32 Index, const FText& Label)
{
	return SNew(SBox).HeightOverride(46.f).Padding(0, 2)
	[
		SNew(SButton).ButtonStyle(FCoreStyle::Get(), "NoBorder")
		.IsFocusable(false).ContentPadding(0.f)
		.OnHovered_Lambda([this, Index] { ActiveOption = Index; })
		.OnClicked_Lambda([this, Index] { return ActivateOption(Index); })
		[
			SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor_Lambda([this, Index]
			{
				return ActiveOption == Index ? FLinearColor(0.13f, 0.3f, 0.43f) : FLinearColor(0.06f, 0.07f, 0.08f);
			}).Padding(FMargin(12, 5)).VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text_Lambda([this, Index, Label]
				{
					return Index == 2 && bConfirmDelete
						? NSLOCTEXT("Inventory", "ConfirmDeleteOne", "Confirm delete (1 item)") : Label;
				}).Font(FCoreStyle::GetDefaultFontStyle("Regular", 19)).ColorAndOpacity(FLinearColor::White)
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
