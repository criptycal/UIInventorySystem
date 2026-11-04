// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HealthBarWidget.h"
#include "Components/WrapBox.h"
#include "UI/HeartWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/Components/HealthComponent.h"

void UHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!HealthBarWrapBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("HealthBarWrapBox is not bound in HealthBarWidget."));
		return;
	}

	HealthBarWrapBox->ClearChildren();
	
	APlayerController* PlayerController = GetOwningPlayer();
	APawn* PlayerPawn = GetOwningPlayerPawn();

	if(!PlayerController || !PlayerPawn || !HeartWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController or PlayerPawn is null in HealthBarWidget."));
		return;
	}

	UHealthComponent* HealthComponent = PlayerPawn->FindComponentByClass<UHealthComponent>();

	if (!HealthComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("HealthComponent not found on PlayerPawn in HealthBarWidget."));
		return;
	}

	const float MaxHealth = HealthComponent->GetMaxHealth();

	if (MaxHealth <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("MaxHealth is less than or equal to zero in HealthBarWidget."));
		return;
	}

	const float CurrentHealth = FMath::Clamp(HealthComponent->GetCurrentHealth(), 0.0f, MaxHealth);

	const int32 HeartCount = FMath::CeilToInt(MaxHealth);

	for (int32 Index = 0; Index < HeartCount; ++Index)
	{
		if (HeartWidgetClass)
		{
			UHeartWidget* HeartWidget = CreateWidget<UHeartWidget>(PlayerController, HeartWidgetClass);
			
			if (!HeartWidget)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to create HeartWidget in HealthBarWidget."));
				continue;
			}
			const float HeartPercent = FMath::Clamp(
				CurrentHealth - static_cast<float>(Index), 0.0f, 1.0f);

			HeartWidget->SetPercent(HeartPercent);
			HealthBarWrapBox->AddChild(HeartWidget);
		}
	}
}
