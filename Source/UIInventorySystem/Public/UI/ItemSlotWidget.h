// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemSlotWidget.generated.h"


class UImage;
class UTextBlock;
class UTexture2D;
/**
 *
 */
UCLASS()
class UIINVENTORYSYSTEM_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetItemDisplay(UTexture2D* InIcon, int32 InQuantity);

protected:
	virtual void  NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> QuantityText;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> DisplayIcon = nullptr;

	int32 DisplayQuantity = 0;

	void RefreshDisplay();
};
