// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CurrencyComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrencyBalanceChanged, int32, NewBalance);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UIINVENTORYSYSTEM_API UCurrencyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCurrencyComponent();

	UFUNCTION(BlueprintPure, Category = "Currency")
	int32 GetBalance() const;

	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool TryAddCurrency(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool TrySpendCurrency(int32 Amount);

	UPROPERTY(BlueprintAssignable, Category = "Currency")
	FOnCurrencyBalanceChanged OnBalanceChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;	

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Currency")
	int32 Balance = 0;

	
};
