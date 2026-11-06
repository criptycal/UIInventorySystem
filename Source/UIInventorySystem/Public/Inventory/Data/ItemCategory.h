#pragma once

#include "CoreMinimal.h"
#include "ItemCategory.generated.h"

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Weapon UMETA(DisplayName = "Weapon"),
	Shields UMETA(DisplayName = "Shields"),
	Food UMETA(DisplayName = "Food")
};
