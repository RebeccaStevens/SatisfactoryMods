#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "PEWS_NavigationHistory.h"

#include "PEWS_Panel.generated.h"

UINTERFACE()
class UPEWS_Panel : public UInterface {
	GENERATED_BODY()
};

/**
 * An interface for all panels that need special navigation handling.
 */
class PLAYEREQUIPMENTWIDGETSWITCHER_API IPEWS_Panel {
	GENERATED_BODY()

public:
	/**
	 * Called when this panel is navigated to.
	 *
	 * @param Data Data for the navigation event.
	 * @param DesiredSize The size the panel should be to prevent a layout shift.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PEWS")
	void OnNavigate(const UPEWS_NavigationHistoryEntryData* Data, const FVector2D DesiredSize);
};
