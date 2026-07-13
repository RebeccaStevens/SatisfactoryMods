#pragma once

#include "CoreMinimal.h"

#include "UserSettings/BGC_BuildModeGroup.h"

#include "BGC_NavigationHistory.generated.h"

/**
 * A navigation history entry's data.
 */
UCLASS(Abstract, BlueprintType, EditInlineNew, Const)
class BUILDGUNCONFIG_API UBGC_NavigationHistoryEntryData : public UObject {
	GENERATED_BODY()
};

/**
 * A navigation history entry's data with a build mode group id.
 */
UCLASS()
class BUILDGUNCONFIG_API UBGC_NavigationHistoryEntryData_BuildModeGroupId : public UBGC_NavigationHistoryEntryData {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, DisplayName = "Make Navigation History Entry Data")
	static UBGC_NavigationHistoryEntryData_BuildModeGroupId* Make(const int32 BuildModeGroupId) {
		auto Value = NewObject<UBGC_NavigationHistoryEntryData_BuildModeGroupId>();
		Value->BuildModeGroupId = BuildModeGroupId;
		return MoveTemp(Value);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BuildModeGroupId;
};

/**
 * Entry data for the navigation history of the widget switcher.
 */
USTRUCT(BlueprintType)
struct FBGC_NavigationHistoryEntry {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBGC_NavigationHistoryEntryData> Data;
};
