#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/Function.h"

#include "Components/ListView.h"
#include "Components/PanelWidget.h"

#include "Units/BBW_ValueWithRawUnit.h"

#include "BBW_Utils.generated.h"

class UPanelSlot;

/**
 * Utility functions for BuildGunConfig.
 */
UCLASS()
class BLUEBEKAWIDGETS_API UBBW_Utils : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	/**
	 * Move an item in an array from one index to another.
	 *
	 * Each item between the from and to index will be shifted in the direction of the move to fill the gap left by the
	 * moved item.
	 *
	 * @param Array The array to modify.
	 * @param FromIndex The index of the item to move.
	 * @param ToIndex The index to move the item to.
	 */
	template<typename TElementType>
	UFUNCTION(BlueprintCallable, Category = "BBW|Array")
	static void MoveItemInArray(UPARAM(ref) TArray<TElementType>& Array, int32 FromIndex, int32 ToIndex);

	/**
	 * Find the closest value in an array to the given value.
	 *
	 * @param Array The sorted array to search.
	 * @param Value The value to find the closest match to.
	 * @return The closest value in the array.
	 *
	 * @remarks The array must be sorted in ascending order.
	 */
	template<typename TElementType>
	static TElementType FindClosestValueInSortedArray(UPARAM(ref) const TArray<TElementType>& Array, const TElementType Value);

	/**
	 * Find the closest value in an array to the given value.
	 *
	 * @param Array The sorted array to search.
	 * @param Value The value to find the closest match to.
	 * @return The closest value in the array.
	 *
	 * @remarks The array must be sorted in ascending order.
	 */
	UFUNCTION(BlueprintPure, DisplayName = "Find Closest Value in Sorted Array (Int)", Category = "BBW|Array")
	static int32 FindClosestValueInSortedIntArray(UPARAM(ref) const TArray<int32>& Array, int32 Value);

	/**
	 * Find the closest value in an array to the given value.
	 *
	 * @param Array The sorted array to search.
	 * @param Value The value to find the closest match to.
	 * @return The closest value in the array.
	 *
	 * @remarks The array must be sorted in ascending order.
	 */
	UFUNCTION(BlueprintPure, DisplayName = "Find Closest Value in Sorted Array (Float)", Category = "BBW|Array")
	static double FindClosestValueInSortedDoubleArray(UPARAM(ref) const TArray<double>& Array, double Value);

	/**
	 * Add a value to an array if it is not already present (within error tolerance).
	 *
	 * @param Array The array to modify.
	 * @param Value The value to add.
	 * @param ErrorTolerance Values within this distance of an existing value will not be added.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Add Unique (Float)", Category = "BBW|Array")
	static void AddUniqueDouble(UPARAM(ref) TArray<double>& Array, double Value, double ErrorTolerance = 1.e-8);

	/**
	 * Add a value to the appropriate index of a sorted array if it is not already present (within error tolerance).
	 *
	 * @param Array The array to modify.
	 * @param Value The value to add.
	 * @param ErrorTolerance Values within this distance of an existing value will not be added.
	 *
	 * @remarks The array must be sorted in ascending order.
	 */
	UFUNCTION(BlueprintCallable, DisplayName = "Add Unique to Sorted Array (Float)", Category = "BBW|Array")
	static void AddUniqueFloatToSorted(UPARAM(ref) TArray<double>& Array, double Value, double ErrorTolerance = 1.e-8);

	/**
	 * Parse a number with a unit suffix.
	 *
	 * @param NumberWithUnitSuffix The text to parse.
	 * @return The parsed number with its unit.
	 *
	 * @remarks If the text does not contain a valid number the result will be 0.
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|String")
	static FBBW_ValueWithRawUnit ParseNumberWithUnitSuffix(UPARAM(ref) const FText& NumberWithUnitSuffix);

	/**
	 * Round a value to the given number of decimal places.
	 *
	 * @param Value The value to round.
	 * @param Precision The number of decimal places to round to.
	 * @param Base The base of the rounding operation.
	 * @returns The rounded value.
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Math")
	static double RoundWithPrecision(double Value, int32 Precision = 2, int32 Base = 10);

	/**
	 * Round a value to the given number of significant digits.
	 *
	 * @param Value The value to round.
	 * @param SignificantDigits The number of significant digits to round to.
	 * @param Base The base of the rounding operation.
	 * @returns The rounded value.
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Math")
	static double RoundSignificantDigits(double Value, int32 SignificantDigits = 2, int32 Base = 10);

	/**
	 * Round a value to the closest multiple of the given value.
	 *
	 * @param Value The value to round.
	 * @param Multiple The multiple to round to.
	 * @returns The rounded value.
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Math")
	static double RoundToClosestMultiple(double Value, double Multiple);

	/**
	 * Normalize a value to a 0-1 range using a logarithmic scale.
	 *
	 * @param Base The base of the logarithm.
	 * @param Value The value to perform the operation on.
	 * @param ExponentRange The range of the exponents of the logarithmic values (must be greater than zero).
	 * @param ExponentOffset The exponent offset of the logarithmic values.
	 *
	 * @remarks The ExponentRange is the difference between the exponents of the minimum and maximum values that the Value can be.
	 * While ExponentOffset is how far off the minimum value's exponent is from 0.<br>
	 * For example, if Base = 10, Minimum = 10, Maximum = 1000, then ExponentRange = 2 and ExponentOffset = -1, as Minimum = 10^1, Maximum=10^3 and 3-1=2 and 0-1=-1.
	 *
	 * @see DenormalizeFromLogRangeWithOffset
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Math", meta = (ReturnDisplayName = "Normalized Logarithmized Value"))
	static double NormalizeToLogRangeWithOffset(double Base, double Value, double ExponentRange, double ExponentOffset);

	/**
	 * Reverse the normalization of a value using a logarithmic scale to recover the original value.
	 *
	 * @param Base The base of the logarithm.
	 * @param NormalizedLogarithmizedValue The value to perform the operation on.
	 * @param ExponentRange The range of the exponents of the logarithmic values (must be greater than zero).
	 * @param ExponentOffset The exponent offset of the logarithmic values.
	 *
	 * @remarks The ExponentRange is the difference between the exponents of the minimum and maximum values that the Value can be.
	 * While ExponentOffset is how far off the minimum value's exponent is from 0.<br>
	 * For example, if Base = 10, Minimum = 10, Maximum = 1000, then ExponentRange = 2 and ExponentOffset = -1, as Minimum = 10^1, Maximum=10^3 and 3-1=2 and 0-1=-1.
	 *
	 * @see NormalizeToLogRangeWithOffset
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Math", meta = (ReturnDisplayName = "Value"))
	static double DenormalizeFromLogRangeWithOffset(double Base, double NormalizedLogarithmizedValue, double ExponentRange, double ExponentOffset);

	/**
	 * Normalize a value to a 0-1 range using a logarithmic scale.
	 *
	 * @param Base The base of the logarithm.
	 * @param Value The value to perform the operation on.
	 * @param Minimum The minimum value.
	 * @param Maximum The maximum value (must be greater than minimum).
	 *
	 * @see DenormalizeFromLogRange
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Math", meta = (ReturnDisplayName = "Normalized Logarithmized Value"))
	static double NormalizeToLogRange(double Base, double Value, double Minimum, double Maximum);

	/**
	 * Reverse the normalization of a value using a logarithmic scale to recover the original value.
	 *
	 * @param Base The base of the exponential.
	 * @param NormalizedLogarithmizedValue The value to perform the operation on.
	 * @param Minimum The minimum value.
	 * @param Maximum The maximum value (must be greater than minimum).
	 *
	 * @see NormalizeToLogRange
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Math", meta = (ReturnDisplayName = "Value"))
	static double DenormalizeFromLogRange(double Base, double NormalizedLogarithmizedValue, double Minimum, double Maximum);

	/**
	 * Normalize a value to a 0-1 range.
	 *
	 * @param Value The value to normalize.
	 * @param Range The range the value can take.
	 * @param Offset How far off the start of the range is from zero.
	 *
	 * @see DenormalizeWithOffset
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Math", meta = (ReturnDisplayName = "Normalized Value"))
	static double NormalizeWithOffset(double Value, double Range, double Offset);

	/**
	 * Reverse the normalization of a value to recover the original value.
	 *
	 * @param NormalizedValue The value to denormalize.
	 * @param Range The range the value can take.
	 * @param Offset How far off the start of the range is from zero.
	 *
	 * @see NormalizeWithOffset
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Math", meta = (ReturnDisplayName = "Value"))
	static double DenormalizeWithOffset(double NormalizedValue, double Range, double Offset);

	/**
	 * Normalize a value to a 0-1 range.
	 *
	 * @param Value The value to normalize.
	 * @param Minimum The minimum value.
	 * @param Maximum The maximum value (must be greater than minimum).
	 *
	 * @see Denormalize
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Math", meta = (ReturnDisplayName = "Normalized Value"))
	static double Normalize(double Value, double Minimum, double Maximum);

	/**
	 * Reverse the normalization of a value to recover the original value.
	 *
	 * @param NormalizedValue The value to denormalize.
	 * @param Minimum The minimum value.
	 * @param Maximum The maximum value (must be greater than minimum).
	 *
	 * @see Normalize
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Math", meta = (ReturnDisplayName = "Value"))
	static double Denormalize(double NormalizedValue, double Minimum, double Maximum);

	/**
	 * Find the first parent slot of the given widget that is an instance of the given slot class.
	 *
	 * @param Widget The widget to get the parent slot of.
	 * @param SlotClass The class of the slot to find.
	 * @return The first parent slot of the given widget that is an instance of the given slot class, or nullptr if none is found.
	 */
	UFUNCTION(BlueprintCallable, Category = "BBW|Widget", meta = (DeterminesOutputType = "SlotClass"))
	static UPanelSlot* FindParentSlotOfClass(UWidget* Widget, const TSubclassOf<UPanelSlot> SlotClass);

	/**
	 * Move an item in a list view from one index to another.
	 *
	 * Each item between the from and to index will be shifted in the direction of the move to fill the gap left by the
	 * moved item.
	 *
	 * @param ListView The list view to move the item in.
	 * @param FromIndex The index of the item to move.
	 * @param ToIndex The index to move the item to.
	 */
	UFUNCTION(BlueprintCallable, Category = "BBW|Widget|ListView")
	static void MoveItemInListView(UListView* ListView, int32 FromIndex, int32 ToIndex);

	/**
	 * Mutate the items of a list view widget.
	 *
	 * @warning The modifier function should not add or remove children from the array, but it can reorder them.
	 *
	 * @param ListView The list view to mutate the items of.
	 * @param Modifier The function to call to modify the items.
	 *
	 * @remarks The given modifier function will be called with a mutable reference to the list view's items.
	 */
	static void ModifyListItems(UListView* ListView, const TFunctionRef<void(TArray<UObject*>&)>& Modifier);

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
	 *
	 * @param ListView The list view to get the items of.
	 * @return The items in the list view.
	 */
	static TArray<UObject*>& GetListItemsUnsafe(const UListView* ListView);

public:
	/**
	 * Reset the order of the items in the list view of a reorder list to their original order.
	 *
	 * @param ListView The list view to reset.
	 */
	UFUNCTION(BlueprintCallable, Category = "BBW|Widget|ReorderList")
	static void ResetReorderListOrder(UListView* ListView);

	/**
	 * Automatically compute a set of snap values for a slider using a linear scale.
	 *
	 * @param SupportedUnits
	 * @param MinValue The minimum value of the range.
	 * @param MaxValue The maximum value of the range.
	 * @param SnapIncrement How far apart snap values should be.
	 * @param ErrorTolerance Values within this distance of an existing value will not be added.
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Widget|Slider")
	static TArray<double> ComputeLinearAutoSnapValues(TSet<UBBW_Unit*> SupportedUnits, double MinValue, double MaxValue, double SnapIncrement = 1, double ErrorTolerance =1.e-8);

	/**
	 * Automatically compute a set of snap values for a slider using a logarithmic scale.
	 *
	 * @param SupportedUnits
	 * @param MinValue The minimum value of the range.
	 * @param MaxValue The maximum value of the range.
	 * @param LogarithmicBase The logarithmic base to use (must be greater than 1).
	 * @param SnapIncrement How far apart the exponents of each snap values should be.
	 * @param SnapSubdivisions How many additional snap values should be created between snap increments.
	 * @param ErrorTolerance Values within this distance of an existing value will not be added.
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Widget|Slider")
	static TArray<double> ComputeLogarithmicAutoSnapValues(TSet<UBBW_Unit*> SupportedUnits, double MinValue, double MaxValue, double LogarithmicBase, double SnapIncrement = 1, int32 SnapSubdivisions = 0, double ErrorTolerance = 0);

protected:
	/**
	 * Automatically compute a set of snap values for a slider.
	 *
	 * @param SupportedUnits
	 * @param MinValue The minimum value of the range.
	 * @param MaxValue The maximum value of the range.
	 * @param LogarithmicBase The logarithmic base to use or a value <= 1 to use a linear scale.
	 * @param SnapIncrement How far apart snap values should be (for logarithmic scale, this is increment of the exponent between snaps).
	 * @param SnapSubdivisions How many additional snap values should be created between snap increments (for linear scales, this is equivalent to decreasing the snap increment).
	 * @param ErrorTolerance Values within this distance of an existing value will not be added.
	 */
	static TArray<double> ComputeAutoSnapValues(TSet<UBBW_Unit*> SupportedUnits, double MinValue, double MaxValue, double LogarithmicBase, double SnapIncrement, int32
	                                            SnapSubdivisions = 0,
	                                            double ErrorTolerance = UE_DOUBLE_SMALL_NUMBER
	);

public:
	/**
	 * Find the unit in the array that best represents the given value.
	 *
	 * Will return a unit where the multiplier is less than or equal to the value if possible.
	 *
	 * @param Units The unit to search.
	 * @param Value The value to find a unit for.
	 * @return The unit that best represents the value or nullptr if not suitable unit was found.
	 */
	UFUNCTION(BlueprintCallable, Category = "BBW|Unit")
	static UBBW_Unit* FindUnitForValue(const TSet<UBBW_Unit*>& Units, double Value);

	/**
	 * Find the unit in the array with the given unit.
	 *
	 * @param Units The units to search.
	 * @param UnitText The unit to search for.
	 * @returns The found unit or nullptr if not unit was found.
	 */
	UFUNCTION(BlueprintCallable, Category = "BBW|Unit")
	static UBBW_Unit* FindUnitByRawUnitText(const TSet<UBBW_Unit*>& Units, FText UnitText);
};
