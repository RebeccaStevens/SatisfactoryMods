#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "BBW_Unit.generated.h"

UCLASS()
class UBBW_Unit : public UDataAsset {
	GENERATED_BODY()

public:
	/**
	 * The unit of this value group.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BBW|Unit")
	FText UnitOnlyText;

	/**
	 * A pattern to display a value with this group's unit.
	 *
	 * @remarks `{Value}` in the text will be replaced with the value to display.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BBW|Unit")
	FText UnitPattern;

	/**
	 * How much the unit of this group differs from the base unit.
	 *
	 * Example: If the base unit is meters and this unit is centimeters, the multiplier would be 0.01.
	 * If the base unit is meters and this unit is kilometers, the multiplier would be 1000.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BBW|Unit", DisplayName = "Value", meta = (ClampMin = "0"))
	double Multiplier = 1.0;

	/**
	 * Construct a new unit.
	 */
	UBBW_Unit() {
		UnitOnlyText = FText::FromString(TEXT(""));
		UnitPattern = FText::FromString(TEXT("{Value}"));
		Multiplier = 1.0;
	}

	/**
	 * Get the default (unitless) unit.
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Unit")
	static UBBW_Unit* GetDefaultUnit() {
		static const FSoftObjectPath DefaultUnitPath(TEXT("/BlueBekaWidgets/Units/BBW_Unit_None.BBW_Unit_None"));

		if (UBBW_Unit* DefaultUnit = Cast<UBBW_Unit>(DefaultUnitPath.TryLoad())) {
			return DefaultUnit;
		}

		checkNoEntry();
		return GetMutableDefault<UBBW_Unit>();
	}

	/**
	 * Returns the default unit if the given unit is invalid.
	 */
	UFUNCTION(BlueprintPure, Category = "BBW|Unit")
	static UBBW_Unit* UnitOrDefault(UBBW_Unit* Unit) {
		return IsValid(Unit) ? Unit : GetDefaultUnit();
	}
};

