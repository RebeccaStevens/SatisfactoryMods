#include "BGC_Utils.h"

#include "CoreMinimal.h"
#include "Templates/Function.h"

UPanelSlot* UBGC_Utils::FindParentSlotOfClass(UWidget* Widget, const TSubclassOf<UPanelSlot> SlotClass) {
  if (!IsValid(Widget)) {
    return nullptr;
  }

  for (UObject* Current = Widget; Current;) {
    UWidget* AsWidget = Cast<UWidget>(Current);
    if (!AsWidget) {
      Current = Current->GetOuter();
      continue;
    }

    UPanelSlot* AsSlot = AsWidget->Slot.Get();
    if (AsSlot && (!SlotClass || AsSlot->IsA(SlotClass))) {
      return AsSlot;
    }

    Current = AsWidget->GetParent();
    if (!Current) {
      Current = AsWidget->GetOuter();
    }
  }

  return nullptr;
}

void UBGC_Utils::SetLocked(UFGInventoryComponent* InventoryComponent, bool Locked) {
  if (!IsValid(InventoryComponent)) {
    return;
  }

  InventoryComponent->SetLocked(Locked);
}

void UBGC_Utils::MoveItemInArray(TArray<UObject*>& Array, int32 FromIndex, int32 ToIndex) {
  if (FromIndex == ToIndex) {
    return;
  }

  int32 NumItems = Array.Num();
  if (FromIndex < 0 || FromIndex >= NumItems || ToIndex < 0 || ToIndex >= NumItems) {
    UE_LOG(
      LogBuildGunConfig,
      Error,
      TEXT("Invalid index range for move item in array. FromIndex: %d, ToIndex: %d, NumItems: %d"),
      FromIndex,
      ToIndex,
      NumItems);
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

void UBGC_Utils::MoveItemInListView(UListView* ListView, int32 FromIndex, int32 ToIndex) {
  if (!ListView || FromIndex == ToIndex) {
    return;
  }

  ModifyListItems(ListView, [&](TArray<UObject*>& Items) { MoveItemInArray(Items, FromIndex, ToIndex); });
}

void UBGC_Utils::ModifyListItems(UListView* ListView, TFunctionRef<void(TArray<UObject*>&)> Modifier) {
  if (!IsValid(ListView)) {
    return;
  }

  TArray<UObject*>& Items = GetListItemsUnsafe(ListView);
  Modifier(Items);
  ListView->RequestRefresh();
}

TArray<UObject*>& UBGC_Utils::GetListItemsUnsafe(UListView* ListView) {
  if (!ListView) {
    static TArray<UObject*> EmptyArray;
    return EmptyArray;
  }

  // Get the items in the list view (and force the type to be mutable).
  return const_cast<TArray<UObject*>&>(ListView->GetListItems());
}

void UBGC_Utils::ResetReorderList(UListView* ListView) {
  if (!IsValid(ListView)) {
    return;
  }
  ModifyListItems(ListView, [&](TArray<UObject*>& EntryData) {
    EntryData.StableSort([](UObject& A, UObject& B) {
      auto GetIndex = [](const UObject& Obj) {
        const UClass* C = Obj.GetClass();
        if (C && C->ImplementsInterface(UBGC_ReorderList_EntryData::StaticClass())) {
          return IBGC_ReorderList_EntryData::Execute_GetOriginalIndex(const_cast<UObject*>(&Obj));
        }
        UE_LOG(
          LogBuildGunConfig, Warning, TEXT("Object %s does not implement IBGC_ReorderList_EntryData."), *Obj.GetName());
        return 0;
      };

      return GetIndex(A) < GetIndex(B);
    });
  });
}

TSubclassOf<AFGHologram> UBGC_Utils::GetHologramClass(const UFGBuildGunStateBuild* state) {
  if (!IsValid(state)) {
    return nullptr;
  }
  auto buildGun = state->GetBuildGun();
  if (!IsValid(buildGun)) {
    return nullptr;
  }
  auto buildGunState = buildGun->GetCurrentState();
  if (!IsValid(buildGunState)) {
    return nullptr;
  }
  auto buildGunStateBuild = Cast<UFGBuildGunStateBuild>(buildGunState);
  if (!IsValid(buildGunStateBuild)) {
    return nullptr;
  }
  auto hologram = buildGunStateBuild->GetHologram();
  if (!IsValid(hologram)) {
    return nullptr;
  }
  return hologram->GetClass();
}

double UBGC_Utils::RoundWithPrecision(double Value, int32 Precision, int32 Base) {
  if (Value == 0.0) {
    return 0.0;
  }

  double Factor = FMath::Pow(Base, (double)Precision);
  return FMath::RoundToInt(Value * Factor) / Factor;
}

double UBGC_Utils::RoundSignificantDigits(double Value, int32 SignificantDigits, int32 Base) {
  if (Value == 0.0) {
    return 0.0;
  }

  double Factor =
    FMath::Pow(Base, (double)(SignificantDigits - FMath::CeilToInt(FMath::LogX(Base, FMath::Abs(Value)))));
  return FMath::RoundToInt(Value * Factor) / Factor;
}

double UBGC_Utils::FindClosestValueInSortedArray(const TArray<double>& Values, double Value) {
  int32 Count = Values.Num();
  if (Count == 0) {
    return NAN;
  }
  if (Count == 1) {
    return Values[0];
  }

  int32 MinIndex = 0;
  int32 MaxIndex = Count - 1;
  while (MaxIndex - MinIndex > 1) {
    int32 MidIndex = (MinIndex + MaxIndex) / 2;
    if (Values[MidIndex] < Value) {
      MinIndex = MidIndex;
    } else {
      MaxIndex = MidIndex;
    }
  }

  double MinValueDistance = FMath::Abs(Value - Values[MinIndex]);
  double MaxValueDistance = FMath::Abs(Value - Values[MaxIndex]);
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
