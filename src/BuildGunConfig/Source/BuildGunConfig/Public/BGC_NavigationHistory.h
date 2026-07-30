#pragma once

#include "CoreMinimal.h"

#include "PEWS_NavigationHistory.h"

#include "UserSettings/BGC_BuildModeGroup.h"

#include "BGC_NavigationHistory.generated.h"

/**
 * A navigation history entry's data with a build mode group id.
 */
UCLASS()
class BUILDGUNCONFIG_API UBGC_NavigationHistoryEntryData_BuildModeGroupId : public UPEWS_NavigationHistoryEntryData {
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
