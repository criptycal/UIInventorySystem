// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptWidget.generated.h"


class AItemPickup;
class UBorder;
class UInteractionComponent;

/**
 * 
 */
UCLASS()
class UIINVENTORYSYSTEM_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> PromptContainer;
	
	UPROPERTY(Transient)
	TObjectPtr<UInteractionComponent> BoundInteractionComponent;

	UFUNCTION()
	void HandleFocusedItemChanged(AItemPickup* NewFocusedItem);
	
};
