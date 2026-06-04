#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/Function.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/ListView.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "Equipment/FGBuildGunBuild.h"
#include "FGBuildGunModeDescriptor.h"
#include "FGInventoryComponent.h"
#include "Hologram/FGHologram.h"

#include "BGC_Module.h"

#include "BGC_Utils.generated.h"

class UPanelSlot;

/**
 * Utility functions for BuildGunConfig.
 */
UCLASS()
class BUILDGUNCONFIG_API UBGC_Utils : public UBlueprintFunctionLibrary {
  GENERATED_BODY()

public:
  /**
   * Find the first parent slot of the given widget that is a child of the given slot class.
   * Returns nullptr if no such slot is found.
   */
  UFUNCTION(BlueprintCallable, Category = "Widget", meta = (DeterminesOutputType = "SlotClass"))
  static UPanelSlot* FindParentSlotOfClass(UWidget* Widget, const TSubclassOf<UPanelSlot> SlotClass);

  /**
   * Lock or unlock the given inventory component, preventing any items from being added or removed.
   */
  UFUNCTION(BlueprintCallable, Category = "Inventory")
  static void SetLocked(UFGInventoryComponent* InventoryComponent, bool Locked);

  /**
   * Move an item in an array from one index to another.
   *
   * Each item between the from and to index will be shifted in the direction of the move to fill the gap left by the
   * moved item.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|Array")
  static void MoveItemInArray(UPARAM(ref) TArray<UObject*>& Array, int32 FromIndex, int32 ToIndex);

  /**
   * Move an item in a list view from one index to another.
   *
   * Each item between the from and to index will be shifted in the direction of the move to fill the gap left by the
   * moved item.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|ListView")
  static void MoveItemInListView(UListView* ListView, int32 FromIndex, int32 ToIndex);

  /**
   * Safely mutate the items of a `UListView` and request a refresh afterwards.
   * This avoids requiring callers to `RequestRefresh()` manually and reduces the risk of
   * forgetting to update the UI after changing the internal array.
   *
   * Usage (C++ only):
   *   UBGC_Utils::ModifyListItems(MyListView, [&](TArray<UObject*>& Items) { });
   */
  static void ModifyListItems(UListView* ListView, TFunctionRef<void(TArray<UObject*>&)> Modifier);

  /**
   * Reset the items in the list view of a reorder list to their original order.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|ReorderList")
  static void ResetReorderList(UListView* ListView);

  /**
   * Get the hologram class associated with a build gun state build.
   *
   * @param state The build gun state to get the hologram class from.
   */
  UFUNCTION(BlueprintPure, Category = "BuildGunConfig|Utils")
  static TSubclassOf<AFGHologram> GetHologramClass(const UFGBuildGunStateBuild* state);

protected:
  /**
   * Get the items in a list view for modification.
   *
   * Items should not be added or removed from the returned array, but the order of items can be modified.
   * Be sure to call RequestRefresh() on the list view after modifying the items to ensure the changes are reflected in
   * the UI.
   *
   * Unsafe: returns a direct mutable reference to the internal array owned by `UListView`.
   * Prefer `ModifyListItems` which guarantees a refresh after mutation.
   */
  static TArray<UObject*>& GetListItemsUnsafe(UListView* ListView);
};
