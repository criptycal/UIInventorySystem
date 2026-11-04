// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIINVENTORYSYSTEM_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// Health is measured in hearts: 1.0 is full, 0.5 is half.
	UPROPERTY(EditDefaultsOnly, Category = "Health", meta = (ClampMin = "1.0", UIMin = "1.0"))
	float MaxHealth = 3.0f;

	UPROPERTY(VisibleInstanceOnly, Category = "Health")
	float CurrentHealth = 0.0f;

};	
