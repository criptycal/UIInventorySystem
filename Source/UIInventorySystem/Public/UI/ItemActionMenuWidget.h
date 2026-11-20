#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemActionMenuWidget.generated.h"

enum class EInventoryItemAction : uint8
{
	Eat,
	Drop,
	Delete,
	Equip
};

DECLARE_DELEGATE_OneParam(FItemActionRequested, EInventoryItemAction);
DECLARE_DELEGATE_OneParam(FItemMenuDismissed, bool /* CloseInventory */);

// A small native modal. Gameplay belongs to InventoryComponent, not this widget.
UCLASS()
class UIINVENTORYSYSTEM_API UItemActionMenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void Configure(const FText& InItemName, bool bInFood);
	void ShowFailure(const FText& Message);
	FItemActionRequested OnActionRequested;
	FItemMenuDismissed OnDismissed;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;

private:
	FText ItemName;
	FText Status;
	bool bFood = false;
	bool bConfirmDelete = false;
	int32 ActiveOption = 0;
	TSharedRef<SWidget> MakeOption(int32 Index, const FText& Label);
	FReply ActivateOption(int32 Index);
	FReply Cancel();
};
