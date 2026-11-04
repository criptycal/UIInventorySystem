// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Components/HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	MaxHealth = FMath::Max(MaxHealth, 1.0f);
	CurrentHealth = MaxHealth;
	// CurrentHealth = FMath::Clamp(2.5, 0.0f, MaxHealth); testing health
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

float UHealthComponent::GetMaxHealth() const
{
	return MaxHealth;
}

float UHealthComponent::GetCurrentHealth() const
{
	return CurrentHealth;
}

