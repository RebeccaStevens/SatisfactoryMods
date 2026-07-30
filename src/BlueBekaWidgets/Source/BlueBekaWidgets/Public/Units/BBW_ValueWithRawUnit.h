#pragma once

#include "CoreMinimal.h"

#include "BBW_ValueWithRawUnit.generated.h"

USTRUCT(BlueprintType)
struct FBBW_ValueWithRawUnit {
	GENERATED_BODY()

	/**
	 * The Value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BBW|Unit")
	double Value = 0.0;

	/**
	 * The Unit of the value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BBW|Unit")
	FText Unit;
};
