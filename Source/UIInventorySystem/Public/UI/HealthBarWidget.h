// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class UWrapBox;
class UHeartWidget;

/**
 * 
 */
UCLASS()
class UIINVENTORYSYSTEM_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> HealthBarWrapBox;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	TSubclassOf<UHeartWidget> HeartWidgetClass;
	
};
