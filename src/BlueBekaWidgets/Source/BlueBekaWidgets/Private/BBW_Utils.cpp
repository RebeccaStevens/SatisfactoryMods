#include "BBW_Utils.h"

#include "CoreMinimal.h"
#include "Templates/Function.h"

#include "BBW_Module.h"
#include "ReorderList/BBW_ReorderList_EntryData.h"
#include "Units/BBW_Unit.h"

template<typename TElementType>
void UBBW_Utils::MoveItemInArray(TArray<TElementType>& Array, const int32 FromIndex, const int32 ToIndex) {
	if (FromIndex == ToIndex) {
		return;
	}

	if (!Array.IsValidIndex(FromIndex) || !Array.IsValidIndex(ToIndex)) {
		UE_LOG(
			LogBBW,
			Error,
			TEXT("Invalid index range for move item in array. FromIndex: %d, ToIndex: %d, NumItems: %d"),
			FromIndex,
			ToIndex,
			Array.Num()
		);
		return;
	}

	TElementType Item = MoveTemp(Array[FromIndex]);
	Array.RemoveAt(FromIndex, EAllowShrinking::No);
	Array.Insert(MoveTemp(Item), ToIndex);
}

template<typename TElementType>
TElementType UBBW_Utils::FindClosestValueInSortedArray(const TArray<TElementType>& Array, const TElementType Value) {
	const int32 Count = Array.Num();
	if (Count == 0) {
		return NAN;
	}
	if (Count == 1) {
		return Array[0];
	}

	int32 MinIndex = 0;
	int32 MaxIndex = Count - 1;
	while (MaxIndex - MinIndex > 1) {
		const int32 MidIndex = (MinIndex + MaxIndex) / 2;
		if (Array[MidIndex] < Value) {
			MinIndex = MidIndex;
		} else {
			MaxIndex = MidIndex;
		}
	}

	const TElementType MinValueDistance = FMath::Abs(Value - Array[MinIndex]);
	const TElementType MaxValueDistance = FMath::Abs(Value - Array[MaxIndex]);
	return MinValueDistance < MaxValueDistance ? Array[MinIndex] : Array[MaxIndex];
}

int32 UBBW_Utils::FindClosestValueInSortedIntArray(const TArray<int32>& Array, const int32 Value) {
	return FindClosestValueInSortedArray(Array, Value);
}

double UBBW_Utils::FindClosestValueInSortedDoubleArray(const TArray<double>& Array, const double Value) {
	return FindClosestValueInSortedArray(Array, Value);
}

void UBBW_Utils::AddUniqueDouble(TArray<double>& Array, double Value, double ErrorTolerance) {
	if (!Array.ContainsByPredicate([&](const auto SavedValue) {
		return FMath::IsNearlyEqual(SavedValue, Value, ErrorTolerance);
	})) {
		Array.Add(Value);
	}
}

void UBBW_Utils::AddUniqueFloatToSorted(TArray<double>& Array, const double Value, const double ErrorTolerance) {
	int32 MinIndex = 0;
	int32 MaxIndex = Array.Num();
	while (MaxIndex > MinIndex) {
		const int32 MidIndex = (MinIndex + MaxIndex) / 2;
		if (FMath::IsNearlyEqual(Value, Array[MidIndex], ErrorTolerance)) {
			return;
		}
		if (Array[MidIndex] < Value) {
			MinIndex = MidIndex + 1;
		} else {
			MaxIndex = MidIndex;
		}
	}

	Array.Insert(Value, MaxIndex);
}

FBBW_ValueWithRawUnit UBBW_Utils::ParseNumberWithUnitSuffix(const FText& NumberWithUnitSuffix) {
	FString StringValue = NumberWithUnitSuffix.ToString();

	// Find the last digit in the string.
	int32 LastDigitIndex = StringValue.Len() - 1;
	while (LastDigitIndex >= 0 && !TChar<TCHAR>::IsDigit(StringValue[LastDigitIndex])) {
		--LastDigitIndex;
	}
	if (LastDigitIndex < 0) {
		return FBBW_ValueWithRawUnit();
	}

	FBBW_ValueWithRawUnit Result;
	Result.Value = FCString::Atod(*StringValue.Mid(0, LastDigitIndex + 1));
	Result.Unit = FText::FromString(StringValue.Mid(LastDigitIndex + 1).TrimStartAndEnd());
	return Result;
}

double UBBW_Utils::RoundWithPrecision(const double Value, const int32 Precision, const int32 Base) {
	if (Value == 0.0) {
		return 0.0;
	}

	const double Factor = FMath::Pow(Base, static_cast<double>(Precision));
	return FMath::RoundToInt(Value * Factor) / Factor;
}

double UBBW_Utils::RoundSignificantDigits(const double Value, const int32 SignificantDigits, const int32 Base) {
	if (Value == 0.0) {
		return 0.0;
	}

	const double Factor =
		FMath::Pow(
			Base,
			static_cast<double>(SignificantDigits - FMath::CeilToInt(FMath::LogX(Base, FMath::Abs(Value))))
		);
	return FMath::RoundToInt(Value * Factor) / Factor;
}

double UBBW_Utils::RoundToClosestMultiple(const double Value, const double Multiple) {
	return FMath::RoundToDouble(Value / Multiple) * Multiple;
}

double UBBW_Utils::NormalizeToLogRangeWithOffset(
	const double Base,
	const double Value,
	const double ExponentRange,
	const double ExponentOffset
) {
	return NormalizeWithOffset(FMath::LogX(Base, Value), ExponentRange, ExponentOffset);
}

double UBBW_Utils::DenormalizeFromLogRangeWithOffset(
	const double Base,
	const double NormalizedLogarithmizedValue,
	const double ExponentRange,
	const double ExponentOffset
) {
	return FMath::Pow(Base, DenormalizeWithOffset(NormalizedLogarithmizedValue, ExponentRange, ExponentOffset));
}

double UBBW_Utils::NormalizeToLogRange(
	const double Base,
	const double Value,
	const double Minimum,
	const double Maximum
) {
	return Normalize(FMath::LogX(Base, Value), FMath::LogX(Base, Minimum), FMath::LogX(Base, Maximum));
}

double UBBW_Utils::DenormalizeFromLogRange(
	const double Base,
	const double NormalizedLogarithmizedValue,
	const double Minimum,
	const double Maximum
) {
	return FMath::Pow(Base, Denormalize(NormalizedLogarithmizedValue, FMath::LogX(Base, Minimum), FMath::LogX(Base, Maximum)));
}

double UBBW_Utils::NormalizeWithOffset(const double Value, const double Range, const double Offset) {
	return (Value + Offset) / Range;
}

double UBBW_Utils::DenormalizeWithOffset(const double NormalizedValue, const double Range, const double Offset) {
	return (NormalizedValue * Range) - Offset;
}

double UBBW_Utils::Normalize(const double Value, const double Minimum, const double Maximum) {
	return NormalizeWithOffset(Value, Maximum - Minimum, -Minimum);
}

double UBBW_Utils::Denormalize(const double NormalizedValue, const double Minimum, const double Maximum) {
	return DenormalizeWithOffset(NormalizedValue, Maximum - Minimum, -Minimum);
}

UPanelSlot* UBBW_Utils::FindParentSlotOfClass(UWidget* Widget, const TSubclassOf<UPanelSlot> SlotClass) {
	if (!IsValid(Widget)) {
		return nullptr;
	}

	for (UObject* Current = Widget; Current;) {
		const UWidget* AsWidget = Cast<UWidget>(Current);
		if (!AsWidget) {
			Current = Current->GetOuter();
			continue;
		}

		if (UPanelSlot* AsSlot = AsWidget->Slot.Get();
			AsSlot && (!SlotClass || AsSlot->IsA(SlotClass))) {
			return AsSlot;
		}

		Current = AsWidget->GetParent();
		if (!Current) {
			Current = AsWidget->GetOuter();
		}
	}

	return nullptr;
}

void UBBW_Utils::MoveItemInListView(UListView* ListView, const int32 FromIndex, const int32 ToIndex) {
	if (!ListView || FromIndex == ToIndex) {
		return;
	}

	ModifyListItems(
		ListView,
		[&](TArray<UObject*>& Items) {
			MoveItemInArray(Items, FromIndex, ToIndex);
		}
	);
}

void UBBW_Utils::ModifyListItems(UListView* ListView, const TFunctionRef<void(TArray<UObject*>&)>& Modifier) {
	if (!IsValid(ListView)) {
		return;
	}

	TArray<UObject*>& Items = GetListItemsUnsafe(ListView);
	Modifier(Items);
	ListView->RequestRefresh();
}

TArray<UObject*>& UBBW_Utils::GetListItemsUnsafe(const UListView* ListView) {
	check(ListView);
	// Get the items in the list view (and force the type to be mutable).
	return const_cast<TArray<UObject*>&>(ListView->GetListItems());
}

void UBBW_Utils::ResetReorderListOrder(UListView* ListView) {
	if (!IsValid(ListView)) {
		return;
	}
	ModifyListItems(
		ListView,
		[&](TArray<UObject*>& EntryData) {
			EntryData.StableSort(
				[](const UObject& A, const UObject& B) {
					auto GetIndex = [](const UObject& Obj) {
						const UClass* C = Obj.GetClass();
						if (C && C->ImplementsInterface(UBBW_ReorderList_EntryData::StaticClass())) {
							return IBBW_ReorderList_EntryData::Execute_GetOriginalIndex(const_cast<UObject*>(&Obj));
						}
						UE_LOG(
							LogBBW,
							Warning,
							TEXT("Object %s does not implement IBBW_ReorderList_EntryData."),
							*Obj.GetName()
						);
						return 0;
					};

					return GetIndex(A) < GetIndex(B);
				}
			);
		}
	);
}

TArray<double> UBBW_Utils::ComputeLinearAutoSnapValues(
	const TSet<UBBW_Unit*> SupportedUnits,
	const double MinValue,
	const double MaxValue,
	const double SnapIncrement,
	const double ErrorTolerance
) {
	return ComputeAutoSnapValues(SupportedUnits, MinValue, MaxValue, 1, SnapIncrement, 0, ErrorTolerance);
}

TArray<double> UBBW_Utils::ComputeLogarithmicAutoSnapValues(
	const TSet<UBBW_Unit*> SupportedUnits,
	const double MinValue,
	const double MaxValue,
	const double LogarithmicBase,
	const double SnapIncrement,
	const int32 SnapSubdivisions,
	const double ErrorTolerance
) {
	check(LogarithmicBase > 1);
	return ComputeAutoSnapValues(SupportedUnits, MinValue, MaxValue, LogarithmicBase, SnapIncrement, SnapSubdivisions, ErrorTolerance);
}

TArray<double> UBBW_Utils::ComputeAutoSnapValues(
	const TSet<UBBW_Unit*> SupportedUnits,
	const double MinValue,
	const double MaxValue,
	const double LogarithmicBase,
	const double SnapIncrement,
	const int32 SnapSubdivisions,
	const double ErrorTolerance
) {
	check(MinValue < MaxValue);
	check(SnapIncrement > 0);
	check(SnapSubdivisions >= 0);
	check(ErrorTolerance >= 0);
	const bool UsingLogarithmicScale = LogarithmicBase > 1;

	TArray<UBBW_Unit*> SortedSupportedUnits = SupportedUnits.Array();
	if (SortedSupportedUnits.Num() == 0) {
		SortedSupportedUnits.Add(UBBW_Unit::GetDefaultUnit());
	}
	Algo::Sort(SortedSupportedUnits, [](const UBBW_Unit* A, const UBBW_Unit* B) {
		return !IsValid(A) ? false : !IsValid(A) ? true : A->Multiplier < B->Multiplier;
	});

	TArray<double> SnapValues;
	SnapValues.Reserve(2 + SortedSupportedUnits.Num() * (SnapSubdivisions + 1));
	AddUniqueFloatToSorted(SnapValues, MinValue, ErrorTolerance);

	// Add snap values for each unit.
	TArray<TArray<double>> UnitSnapValuesGroups;
	UnitSnapValuesGroups.Reserve(SortedSupportedUnits.Num());
	double Min = MinValue;
	double Max = MaxValue;
	if (UsingLogarithmicScale) {
		Min = FMath::LogX(LogarithmicBase, Min);
		Max = FMath::LogX(LogarithmicBase, Max);
	}

	for (int32 UnitGroupIndex = 0; UnitGroupIndex < SortedSupportedUnits.Num(); UnitGroupIndex++) {
		auto& UnitSnapValues = UnitSnapValuesGroups.AddDefaulted_GetRef();
		const auto CurrentUnitGroup = SortedSupportedUnits[UnitGroupIndex];
		const auto NextUnitGroup = SortedSupportedUnits.IsValidIndex(UnitGroupIndex + 1)
			? SortedSupportedUnits[UnitGroupIndex + 1]
			: nullptr;

		auto NextSnapValue = CurrentUnitGroup->Multiplier;
		auto MaxSnapValue = NextUnitGroup == nullptr ? MaxValue: NextUnitGroup->Multiplier;

		NextSnapValue = FMath::Min(NextSnapValue, MaxValue);
		MaxSnapValue = FMath::Min(MaxSnapValue, MaxValue);

		if (UsingLogarithmicScale) {
			NextSnapValue = FMath::LogX(LogarithmicBase, NextSnapValue);
			MaxSnapValue = FMath::LogX(LogarithmicBase, MaxSnapValue);
		}

		while (NextSnapValue < MaxSnapValue || FMath::IsNearlyEqual(NextSnapValue, MaxSnapValue, ErrorTolerance)) {
			if (NextSnapValue > Min || FMath::IsNearlyEqual(NextSnapValue, Min, ErrorTolerance)) {
				const auto RealNextSnapValue = UsingLogarithmicScale ? FMath::Pow(LogarithmicBase, NextSnapValue) : NextSnapValue;
				UnitSnapValues.Add(RealNextSnapValue);
				AddUniqueFloatToSorted(SnapValues, RealNextSnapValue, ErrorTolerance);
			}
			NextSnapValue += SnapIncrement;
		}
	}

	// Subdivide each unit group's snap values.
	if (UnitSnapValuesGroups.Num() > 0 && SnapSubdivisions > 0) {
		for (auto& UnitGroupSnapValue : UnitSnapValuesGroups) {
			for (int32 UnitGroupSnapValueIndex = 1; UnitGroupSnapValueIndex < UnitGroupSnapValue.Num(); UnitGroupSnapValueIndex++) {
				const auto Low = UnitGroupSnapValue[UnitGroupSnapValueIndex - 1];
				const auto High = UnitGroupSnapValue[UnitGroupSnapValueIndex];

				for (int32 Subdivision = 0; Subdivision < SnapSubdivisions; Subdivision++) {
					const auto SubdivisionValue = FMath::Lerp(Low, High, (Subdivision + 1) / (SnapSubdivisions + 1.0));
					AddUniqueFloatToSorted(SnapValues, SubdivisionValue, ErrorTolerance);
				}
			}
		}
	}

	AddUniqueFloatToSorted(SnapValues, MaxValue, ErrorTolerance);
	SnapValues.Shrink();
	checkCode(
		for (int32 i = 1; i < SnapValues.Num(); i++) {
		checkf(SnapValues[i - 1] < SnapValues[i], TEXT("SnapValues is not sorted."));
		}
	);
	return SnapValues;
}

UBBW_Unit* UBBW_Utils::FindUnitForValue(const TSet<UBBW_Unit*>& Units, const double Value) {
	if (Units.IsEmpty()) {
		return nullptr;
	}
	UBBW_Unit* ClosestUnit = nullptr;
	auto ClosestUnitDistance = TNumericLimits<double>::Max();
	auto PositiveFound = false;
	for (auto& Unit : Units) {
		const auto DistanceToGroup = Value - Unit->Multiplier;
		PositiveFound = PositiveFound || DistanceToGroup >= 0;
		if (PositiveFound) {
			if (DistanceToGroup >= 0 && DistanceToGroup < ClosestUnitDistance) {
				ClosestUnitDistance = DistanceToGroup;
				ClosestUnit = Unit;
			}
		} else {
			if (ClosestUnit == nullptr || DistanceToGroup > ClosestUnitDistance) {
				ClosestUnitDistance = DistanceToGroup;
				ClosestUnit = Unit;
			}
		}
	}
	return ClosestUnit;
}

UBBW_Unit* UBBW_Utils::FindUnitByRawUnitText(const TSet<UBBW_Unit*>& Units, const FText UnitText) {
	for (auto& Unit : Units) {
		if (Unit->UnitOnlyText.EqualToCaseIgnored(UnitText)) {
			return Unit;
		}
	}
	return nullptr;
}
