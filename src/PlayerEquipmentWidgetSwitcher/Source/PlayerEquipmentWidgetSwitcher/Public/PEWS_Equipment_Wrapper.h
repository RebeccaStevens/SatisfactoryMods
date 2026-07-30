#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "UI/FGWidgetSwitcher.h"

#include "PEWS_NavigationHistory.h"

#include "PEWS_Equipment_Wrapper.generated.h"

UCLASS(Abstract)
class PLAYEREQUIPMENTWIDGETSWITCHER_API UPEWS_Equipment_Wrapper : public UUserWidget {
	GENERATED_BODY()

protected:
	/**
	 * The navigation history of the widget switcher.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadWrite,
		Category = "PEWS",
		meta = (EditCondition = false, EditConditionHides)
	)
	TArray<FPEWS_NavigationHistoryEntry> NavigationHistory;

	/**
	 * The forward navigation history of the widget switcher.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadWrite,
		Category = "PEWS",
		meta = (EditCondition = false, EditConditionHides)
	)
	TArray<FPEWS_NavigationHistoryEntry> NavigationForwardHistory;

	/**
	 * The first entry in the navigation history.
	 *
	 * This will be the widget that is first displayed when the widget switcher is opened.
	 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "PEWS"
	)
	FPEWS_NavigationHistoryEntry DefaultWidgetHistoryEntry;

	virtual void NativeConstruct() override;

public:
	/**
	 * Adds a widget to the widget switcher.
	 *
	 * @return The index of the added widget.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEWS", meta = (ReturnDisplayName = "Child Index"))
	virtual int32 AddChild(UWidget* Widget);

	/**
	 * Resets the navigation history to the default widget.
	 *
	 * @param PerformNavigation If true, the switcher will navigate to the default widget.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEWS")
	virtual void ResetNavigationHistory(const bool PerformNavigation = true);

	/**
	 * Navigates to a new given history entry.
	 */
	void NavigateTo(const FPEWS_NavigationHistoryEntry& Entry);

	/**
	 * Navigates to a new given history entry.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEWS")
	virtual void NavigateTo(const int32 Index, UPEWS_NavigationHistoryEntryData* Data);

	/**
	 * Navigates back in the navigation history.
	 *
	 * @return True if the navigation was successful, false there was nothing to go back to.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEWS")
	virtual bool NavigateBack();

	/**
	 * Navigates forward in the navigation history.
	 *
	 * @return True if the navigation was successful, false there was nothing to go forward to.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEWS")
	virtual bool NavigateForward();

protected:
	/**
	 * Gets the widget switcher of this widget.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PEWS")
	UFGWidgetSwitcher* GetWidgetSwitcher();
	virtual UFGWidgetSwitcher* GetWidgetSwitcher_Implementation() {
		checkNoEntry();
		return nullptr;
	}

	/**
	 * Get the desired size of a child widget to prevent a layout shift.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PEWS")
	FVector2D GetDesiredChildSize(UWidget* ChildWidget);
	virtual FVector2D GetDesiredChildSize_Implementation(UWidget* ChildWidget) {
		checkNoEntry();
		return FVector2D::ZeroVector;
	}

	/**
	 * Sets the active child of the widget switcher.
	 *
	 * This is called when a navigation event occurs.
	 */
	UFUNCTION(BlueprintCallable, Category = "PEWS")
	virtual void SetActiveChild(const FPEWS_NavigationHistoryEntry& Entry);
};
