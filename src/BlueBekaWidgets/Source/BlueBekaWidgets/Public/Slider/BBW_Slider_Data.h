#pragma once

#include "CoreMinimal.h"

#include "Units/BBW_Unit.h"

#include "BBW_Slider_Data.generated.h"

UENUM(BlueprintType)
enum class EBBW_Slider_ScaleType : uint8 {
	Linear = 0 UMETA(DisplayName = "Linear"),
	Logarithmic = 1 UMETA(DisplayName = "Logarithmic"),
};

USTRUCT(BlueprintType)
struct BLUEBEKAWIDGETS_API FBBW_Slider_Data {
	GENERATED_BODY()

	/**
	 * The value set by default when the slider isn't set up to reflect an option value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value", DisplayName = "Default Value")
	double DefaultValue = 0;

	/**
	 * The minimum difference allowed between valid value.
	 *
	 * Note: This does not directly affect the slider, but rather explicitly set values.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value")
	double ValueStepSize = UE_KINDA_SMALL_NUMBER;

	/**
	 * Defines the mathematical scaling logic applied to the slider's value calculation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale")
	EBBW_Slider_ScaleType ScaleType = EBBW_Slider_ScaleType::Linear;

	/**
	 * Represents the base value used in logarithmic scaling calculations.
	 *
	 * The logarithmic base must be greater than 1.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", DisplayName = "Logarithmic Base", meta = (EditCondition = "ScaleType==EBBW_Slider_ScaleType::Logarithmic", EditConditionHides = "true", ClampMin = "1.0"))
	double LogarithmicBase = 2;

	/**
	 * Determines whether zero is allowed as a valid selectable value.
	 *
	 * Note: This will automatically adjust the slider without needing to set Override Min Slider Value.
	 * To show zero without allowing it to be selected, use Override Min Slider Value instead.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", DisplayName = "Allow Zero", meta = (EditCondition = "ScaleType==EBBW_Slider_ScaleType::Logarithmic", EditConditionHides = "true"))
	bool LogarithmicAllowZero = false;

	/**
	 * The value that should be used to represent zero.
	 * Any value less than or equal to this value will be treated as zero.
	 *
	 * @details If this value is zero, a safe value will be automatically calculated as the exponent of the min value minus one.
	 * @remarks Zero cannot be added directly to a logarithmic scale (as `log(0) = -∞`).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scale", DisplayName = "Zero Value", meta = (EditCondition = "ScaleType==EBBW_Slider_ScaleType::Logarithmic && LogarithmicAllowZero", EditConditionHides = "true", ClampMin = "0"))
	double LogarithmicZeroValue = 0;

	/**
	 * Should the slider snap to particular values.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider", DisplayName = "Enable Slider Snapping", meta = (CategoryHeader = "Snapping"))
	bool EnableSliderSnapping = true;

	/**
	 * The step size for the slider, defining the interval between valid slider values.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Snapping", DisplayName = "Slider Step Size", meta = (ClampMin = "0"))
	double SliderSnappingSize = 1;

	/**
	 * How many times each snapping value should be subdivided.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Snapping", DisplayName = "Logarithmic Snapping Subdivides", meta = (ClampMin = "0", EditCondition = "ScaleType==EBBW_Slider_ScaleType::Logarithmic", EditConditionHides = "true"))
	int32 LogarithmicSnappingSubdivides;

	/**
	 * Should the slider automatically compute suitable snapping values?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Snapping", DisplayName = "Enable Automatic Snapping Values")
	bool EnableAutomaticSnappingValues = true;

	/**
	 * Any non-automatic snapping values that should be enabled.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Snapping", DisplayName = "Additional Snapping Values")
	TSet<double> UserSnappingValues;

	/**
	 * Should the minimum value that can be explicitly set, be changed from the minimum value?
	 *
	 * Note: This would affect values entered via the text box and when the value is programmatically set.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value", DisplayName = "Override Min Explicit Value", meta = (CategoryHeader = "MinExplicit"))
	bool OverrideMinExplicitValue;

	/**
	 * The minimum value that can be explicitly set.
	 *
	 * Note: This affects values entered via the text box and when the value is programmatically set.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value|MinExplicit", DisplayName = "Min Explicit Value")
	double MinExplicitValueOverride = 0;

	/**
	 * Should the maximum value that can be explicitly set, be changed from the maximum value?
	 *
	 * Note: This would affect values entered via the text box and when the value is programmatically set.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value", DisplayName = "Override Max Explicit Value", meta = (CategoryHeader = "MaxExplicit"))
	bool OverrideMaxExplicitValue;

	/**
	 * The maximum value that can be explicitly set.
	 *
	 * Note: This affects values entered via the text box and when the value is programmatically set.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Value|MaxExplicit", DisplayName = "Max Explicit Value")
	double MaxExplicitValueOverride = 0;

	/**
	 * Should the minimum displayed value on the slider, be changed from the minimum valid value?
	 *
	 * Note: This will not affect the minimum value that can be set by the slider, just what is displayed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider", DisplayName = "Override Min Slider Value to Display", meta = (CategoryHeader = "MinSlider"))
	bool OverrideMinSliderValue;

	/**
	 * The minimum value displayed on the slider.
	 *
	 * Note: This does not affect the minimum value that can be set by the slider, just what is displayed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|MinSlider", DisplayName = "Min Slider Value to Display")
	double MinSliderValueOverride = 0;

	/**
	 * Should the maximum displayed value on the slider, be changed from the minimum valid value?
	 *
	 * Note: This will not affect the maximum value that can be set by the slider, just what is displayed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider", DisplayName = "Override Max Slider Value to Display", meta = (CategoryHeader = "MaxSlider"))
	bool OverrideMaxSliderValue;

	/**
	 * The maximum value displayed on the slider.
	 *
	 * Note: This does not affect the maximum value that can be set by the slider, just what is displayed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|MaxSlider", DisplayName = "Max Slider Value to Display")
	double MaxSliderValueOverride = 0;

	/**
	 * The minimum number of significant digits to display.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formatting|Value", DisplayName = "Min Significant Digits to Display", meta = (ClampMin = "1"))
	int32 MinSignificantDigitsToDisplay = 1;

	/**
	 * At least this many digits after the decimal point will be displayed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formatting|Value", DisplayName = "Min Fractional Digits to Display", meta = (ClampMin = "0"))
	int32 MinFractionalDigitsToDisplay = 0;

	/**
	 * The supported units for the value.
	 *
	 * @remark If empty, will be treated the same as if it only contained the default (unitless) unit.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formatting|Unit", DisplayName = "Supported Units")
	TSet<TObjectPtr<UBBW_Unit>> SupportedUnits;

	/**
	 * The unit used when one is not specified.
	 *
	 * For example, when the users inputs "1.23" instead of "1.23 cm".
	 *
	 * @remark If not set, will default to the default (unitless) unit.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formatting|Unit", DisplayName = "Default Unit")
	TObjectPtr<UBBW_Unit> DefaultUnit;

	/**
	 * The unit used when the value is zero.
	 *
	 * @remark If not set, will default to the default (unitless) unit.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formatting|Unit", DisplayName = "Unit for Zero")
	TObjectPtr<UBBW_Unit> UnitForZero;
};
