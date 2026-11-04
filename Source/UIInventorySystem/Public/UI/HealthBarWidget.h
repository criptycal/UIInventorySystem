// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class UWrapBox;
class UHeartWidget;
class UHealthComponent;

/**
 * 
 */
UCLASS()
class UIINVENTORYSYSTEM_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWrapBox> HealthBarWrapBox;

	UPROPERTY(EditDefaultsOnly, Category = "Health")
	TSubclassOf<UHeartWidget> HeartWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UHealthComponent> BoundHealthComponent;

	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float MaxHealth);


	
};
