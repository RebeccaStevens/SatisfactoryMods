#include "BGC_Utils.h"

#include "CoreMinimal.h"
#include "ReorderList/BGC_ReorderList_EntryData.h"
#include "Templates/Function.h"

#include "BGC_Module.h"

UPanelSlot* UBGC_Utils::FindParentSlotOfClass(UWidget* Widget, const TSubclassOf<UPanelSlot> SlotClass) {
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

void UBGC_Utils::SetLocked(UFGInventoryComponent* InventoryComponent, const bool Locked) {
	if (!IsValid(InventoryComponent)) {
		return;
	}

	InventoryComponent->SetLocked(Locked);
}

void UBGC_Utils::MoveItemInArray(TArray<UObject*>& Array, const int32 FromIndex, const int32 ToIndex) {
	if (FromIndex == ToIndex) {
		return;
	}

	if (const int32 NumItems = Array.Num();
		FromIndex < 0 || FromIndex >= NumItems || ToIndex < 0 || ToIndex >= NumItems) {
		UE_LOG(
			LogBuildGunConfig,
			Error,
			TEXT("Invalid index range for move item in array. FromIndex: %d, ToIndex: %d, NumItems: %d"),
			FromIndex,
			ToIndex,
			NumItems
		);
		return;
	}

	UObject* Item = Array[FromIndex];
	if (FromIndex < ToIndex) {
		for (int32 i = FromIndex; i < ToIndex; i++) {
			Array[i] = Array[i + 1];
		}
	} else {
		for (int32 i = FromIndex; i > ToIndex; i--) {
			Array[i] = Array[i - 1];
		}
	}
	Array[ToIndex] = Item;
}

void UBGC_Utils::MoveItemInListView(UListView* ListView, const int32 FromIndex, const int32 ToIndex) {
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

void UBGC_Utils::ModifyListItems(UListView* ListView, const TFunctionRef<void(TArray<UObject*>&)>& Modifier) {
	if (!IsValid(ListView)) {
		return;
	}

	TArray<UObject*>& Items = GetListItemsUnsafe(ListView);
	Modifier(Items);
	ListView->RequestRefresh();
}

void UBGC_Utils::ResetReorderList(UListView* ListView) {
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
						if (C && C->ImplementsInterface(UBGC_ReorderList_EntryData::StaticClass())) {
							return IBGC_ReorderList_EntryData::Execute_GetOriginalIndex(const_cast<UObject*>(&Obj));
						}
						UE_LOG(
							LogBuildGunConfig,
							Warning,
							TEXT("Object %s does not implement IBGC_ReorderList_EntryData."),
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

TSubclassOf<AFGHologram> UBGC_Utils::GetHologramClass(const UFGBuildGunStateBuild* State) {
	if (!IsValid(State)) {
		return nullptr;
	}
	const auto BuildGun = State->GetBuildGun();
	if (!IsValid(BuildGun)) {
		return nullptr;
	}
	const auto BuildGunState = BuildGun->GetCurrentState();
	if (!IsValid(BuildGunState)) {
		return nullptr;
	}
	const auto BuildGunStateBuild = Cast<UFGBuildGunStateBuild>(BuildGunState);
	if (!IsValid(BuildGunStateBuild)) {
		return nullptr;
	}
	const auto Hologram = BuildGunStateBuild->GetHologram();
	if (!IsValid(Hologram)) {
		return nullptr;
	}
	return Hologram->GetClass();
}

double UBGC_Utils::RoundWithPrecision(const double Value, const int32 Precision, const int32 Base) {
	if (Value == 0.0) {
		return 0.0;
	}

	const double Factor = FMath::Pow(Base, static_cast<double>(Precision));
	return FMath::RoundToInt(Value * Factor) / Factor;
}

double UBGC_Utils::RoundSignificantDigits(const double Value, const int32 SignificantDigits, const int32 Base) {
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

double UBGC_Utils::FindClosestValueInSortedArray(const TArray<double>& Values, const double Value) {
	const int32 Count = Values.Num();
	if (Count == 0) {
		return NAN;
	}
	if (Count == 1) {
		return Values[0];
	}

	int32 MinIndex = 0;
	int32 MaxIndex = Count - 1;
	while (MaxIndex - MinIndex > 1) {
		const int32 MidIndex = (MinIndex + MaxIndex) / 2;
		if (Values[MidIndex] < Value) {
			MinIndex = MidIndex;
		} else {
			MaxIndex = MidIndex;
		}
	}

	const double MinValueDistance = FMath::Abs(Value - Values[MinIndex]);
	const double MaxValueDistance = FMath::Abs(Value - Values[MaxIndex]);
	return MinValueDistance < MaxValueDistance ? Values[MinIndex] : Values[MaxIndex];
}

FUnitValue UBGC_Utils::ParseNumberWithUnitSuffix(const FText& NumberWithUnitSuffix) {
	FString StringValue = NumberWithUnitSuffix.ToString();

	// Find the last digit in the string.
	int32 LastDigitIndex = StringValue.Len() - 1;
	while (LastDigitIndex >= 0 && !TChar<TCHAR>::IsDigit(StringValue[LastDigitIndex])) {
		--LastDigitIndex;
	}
	if (LastDigitIndex < 0) {
		return FUnitValue(0.0, StringValue);
	}

	FUnitValue Result;
	Result.Value = FCString::Atod(*StringValue.Mid(0, LastDigitIndex + 1));
	Result.Unit = StringValue.Mid(LastDigitIndex + 1).TrimStartAndEnd();
	return Result;
}

TArray<UObject*>& UBGC_Utils::GetListItemsUnsafe(const UListView* ListView) {
	check(ListView);
	// Get the items in the list view (and force the type to be mutable).
	return const_cast<TArray<UObject*>&>(ListView->GetListItems());
}
