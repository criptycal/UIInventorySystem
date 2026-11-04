// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Components/CurrencyComponent.h"

// Sets default values for this component's properties
UCurrencyComponent::UCurrencyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UCurrencyComponent::BeginPlay()
{
	Super::BeginPlay();
}

int32 UCurrencyComponent::GetBalance() const
{
	return Balance;
}

bool UCurrencyComponent::TryAddCurrency(int32 Amount)
{
	if (Amount <= 0)
	{
		return false;
	}

	if (Amount > MAX_int32 - Balance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot add currency: would cause overflow."));
		return false;
	}

	Balance += Amount;
	OnBalanceChanged.Broadcast(Balance);
	return true;
}

bool UCurrencyComponent::TrySpendCurrency(int32 Amount)
{
	if (Amount <= 0)
	{
		return false;
	}
	if (Balance < Amount)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot spend currency: insufficient balance."));
		return false;
	}
	Balance -= Amount;
	OnBalanceChanged.Broadcast(Balance);
	return true;
}



