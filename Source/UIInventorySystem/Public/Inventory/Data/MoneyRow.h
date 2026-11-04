#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MoneyRow.generated.h"

class UStaticMesh;

USTRUCT(BlueprintType)
struct UIINVENTORYSYSTEM_API FMoneyRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	int32 Amount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
	TObjectPtr<UStaticMesh> Mesh = nullptr;


};
