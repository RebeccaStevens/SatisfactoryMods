#pragma once

#include "CoreMinimal.h"

#include "PEWS_NavigationHistory.generated.h"

/**
 * A navigation history entry's data.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, Const)
class PLAYEREQUIPMENTWIDGETSWITCHER_API UPEWS_NavigationHistoryEntryData : public UObject {
	GENERATED_BODY()
};

/**
 * Entry data for the navigation history of the widget switcher.
 */
USTRUCT(BlueprintType)
struct FPEWS_NavigationHistoryEntry {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UPEWS_NavigationHistoryEntryData> Data;
};
