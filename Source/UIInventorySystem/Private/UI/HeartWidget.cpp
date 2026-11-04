// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HeartWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UHeartWidget::SetPercent(float InPercent)
{
	Percent = FMath::Clamp(InPercent, 0.0f, 1.0f);
	RefreshHeartTexture();
}

void UHeartWidget::RefreshHeartTexture()
{
	if (!HeartImage) return;

	UTexture2D* SelectedTexture = nullptr;

	if (Percent >= 1.0f)
	{
		SelectedTexture = FullHeartTexture;
	}
	else if (Percent >= 0.5f)
	{
		SelectedTexture = HalfHeartTexture;
	}
	else
	{
		SelectedTexture = EmptyHeartTexture;
	}
	HeartImage->SetBrushFromTexture(SelectedTexture, false);
}

void UHeartWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	RefreshHeartTexture();
}