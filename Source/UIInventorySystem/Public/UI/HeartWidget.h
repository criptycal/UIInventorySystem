// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HeartWidget.generated.h"

class UImage;
class UTexture2D;

/**
 * 
 */
UCLASS()
class UIINVENTORYSYSTEM_API UHeartWidget : public UUserWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(meta = (BindWidget))	
	TObjectPtr<UImage> HeartImage;	

	UPROPERTY(EditAnywhere, Category = "Heart", 
		meta = (ClampMin = "0", ClampMax = "1",
		UIMin = "0.0", UIMax = "1.0"))
	float Percent = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Heart")
	TObjectPtr<UTexture2D> FullHeartTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Heart")	
	TObjectPtr<UTexture2D> HalfHeartTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Heart")
	TObjectPtr<UTexture2D> EmptyHeartTexture;

	void RefreshHeartTexture();
public:
	void SetPercent(float InPercent);

protected:
	virtual void NativePreConstruct() override;


};
