#pragma once

#include "CoreMinimal.h"
#include "IDetailPropertyRow.h"
#include "IPropertyTypeCustomization.h"
#include "Slider/BBW_Slider_Data.h"

class FBBWE_Slider_Data_Customization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	static EBBW_Slider_ScaleType GetScaleType(TSharedPtr<IPropertyHandle> PropertyHandle);

	static void CustomizeChild_ScaleType(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> ChildHandle, IDetailPropertyRow& PropertyRow, IPropertyTypeCustomizationUtils& CustomizationUtils);
	static void CustomizeChild_MinMaxValue(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> ChildHandle, IDetailPropertyRow& PropertyRow, IPropertyTypeCustomizationUtils& CustomizationUtils);
	static void CustomizeChild_MaxFractionalDigitsToDisplay(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> ChildHandle, IDetailPropertyRow& PropertyRow, IPropertyTypeCustomizationUtils& CustomizationUtils);
	static void CustomizeChild_LogarithmicBase(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> ChildHandle, IDetailPropertyRow& PropertyRow, IPropertyTypeCustomizationUtils& CustomizationUtils);
	static void CustomizeChild_SliderSnappingSize(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> ChildHandle, IDetailPropertyRow& PropertyRow, IPropertyTypeCustomizationUtils& CustomizationUtils);
	static void CustomizeChild_SliderValueOverride(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> ChildHandle, IDetailPropertyRow& PropertyRow, IPropertyTypeCustomizationUtils& CustomizationUtils);

	static void OnChanged_ScaleType(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> ScaleTypeHandle);
	static void OnChanged_LogarithmicBase(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> LogarithmicBaseHandle);
	static void OnChanged_SliderSnappingSize(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> SliderSnappingSizeHandle);
	static void OnChanged_SliderValue(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> SliderValueHandle);
};
