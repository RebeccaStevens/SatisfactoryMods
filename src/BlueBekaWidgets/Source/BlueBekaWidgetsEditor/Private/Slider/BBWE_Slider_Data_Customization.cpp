#include "Slider/BBWE_Slider_Data_Customization.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "PropertyHandle.h"
#include "Widgets/Text/STextBlock.h"

#include "UI/FGOptionsValueController.h"

#include "Slider/BBW_Slider_Data.h"

#include "BBWE_Module.h"

TSharedRef<IPropertyTypeCustomization> FBBWE_Slider_Data_Customization::MakeInstance() {
	return MakeShareable(new FBBWE_Slider_Data_Customization());
}

void FBBWE_Slider_Data_Customization::CustomizeHeader(
	TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils
) {
}

void FBBWE_Slider_Data_Customization::CustomizeChildren(
	TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils
) {
	uint32 ChildCount = 0;
	PropertyHandle->GetNumChildren(ChildCount);

	struct FGroupData {
		TArray<TSharedPtr<IPropertyHandle>> ChildHandles;
		TMap<FString, TSharedPtr<FGroupData>> Subgroups;
		TSharedPtr<IPropertyHandle> HeaderProperty;
	};

	TSharedPtr<FGroupData> RootGroupData = MakeShared<FGroupData>();

	auto FindOrAddGroupByCategory = [RootGroupData](FString CategoryPath) -> TSharedPtr<FGroupData> {
		FString Left, Right;
		TSharedPtr<FGroupData> CurrentGroup = RootGroupData;

		while (CategoryPath.Split(TEXT("|"), &Left, &Right)) {
			TSharedPtr<FGroupData>& Subgroup = CurrentGroup->Subgroups.FindOrAdd(Left);
			if (!Subgroup.IsValid()) {
				Subgroup = MakeShared<FGroupData>();
			}
			CurrentGroup = Subgroup;
			CategoryPath = Right;
		}

		if (!CategoryPath.IsEmpty()) {
			TSharedPtr<FGroupData>& Subgroup = CurrentGroup->Subgroups.FindOrAdd(CategoryPath);
			if (!Subgroup.IsValid()) {
				Subgroup = MakeShared<FGroupData>();
			}
			CurrentGroup = Subgroup;
		}
		return CurrentGroup;
	};

	// 1. Sort each child property into groups
	for (uint32 i = 0; i < ChildCount; i++) {
		TSharedPtr<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(i);
		if (!ChildHandle.IsValid() || !ChildHandle->IsValidHandle()) {
			continue;
		}

		const FString CategoryPath = ChildHandle->GetDefaultCategoryName().ToString();

		// Check if this property is a header property for a category subgroup
		if (ChildHandle->HasMetaData(TEXT("CategoryHeader"))) {
			FString TargetGroup = ChildHandle->GetMetaData(TEXT("CategoryHeader"));
			if (!TargetGroup.IsEmpty()) {
				TSharedPtr<FGroupData> HeaderForGroup = FindOrAddGroupByCategory(CategoryPath.IsEmpty()
					? TargetGroup
					: CategoryPath + TEXT("|") + TargetGroup);

				if (HeaderForGroup.IsValid()) {
					HeaderForGroup->HeaderProperty = ChildHandle;
					continue; // Do not add header property as a standard child row
				}
			}
		}

		const TSharedPtr<FGroupData> CurrentGroup = FindOrAddGroupByCategory(CategoryPath);
		CurrentGroup->ChildHandles.Add(ChildHandle);
	}

	// Add outer properties too.
	const TSharedPtr<IPropertyHandle> ParentHandle = PropertyHandle->GetParentHandle();
	if (ParentHandle.IsValid()) {
		const TSharedPtr<IPropertyHandle> GrandParentHandle = ParentHandle->GetParentHandle();
		if (GrandParentHandle.IsValid()) {
			const TSharedPtr<IPropertyHandle> MinValHandle = GrandParentHandle->GetChildHandle("mMinVal");
			const TSharedPtr<IPropertyHandle> MaxValHandle = GrandParentHandle->GetChildHandle("mMaxVal");
			if (MinValHandle.IsValid() && MaxValHandle.IsValid()) {
				const auto ValueGroup = FindOrAddGroupByCategory("Value");
				ValueGroup->ChildHandles.Insert(MinValHandle, 0);
				ValueGroup->ChildHandles.Insert(MaxValHandle, 1);
			}

			const TSharedPtr<IPropertyHandle> MaxFractionalDigits = GrandParentHandle->GetChildHandle("MaxFractionalDigitsToDisplay");
			if (MaxFractionalDigits.IsValid()) {
				const auto FormattingValueGroup = FindOrAddGroupByCategory("Formatting|Value");
				FormattingValueGroup->ChildHandles.Insert(MaxFractionalDigits, 2);
			}
		}
	}

	struct FGroupStackElement {
		TSharedPtr<FGroupData> GroupData;
		IDetailGroup* DetailGroup = nullptr;
	};

	TArray<FGroupStackElement> GroupStack;
	GroupStack.Add(FGroupStackElement{ RootGroupData, nullptr });

	// 2. Add properties & groups to the details panel
	while (!GroupStack.IsEmpty()) {
		const auto [GroupData, DetailGroup] = GroupStack.Pop();

		// Process properties at this group level
		for (auto& ChildHandle : GroupData->ChildHandles) {
			IDetailPropertyRow& PropertyRow = (DetailGroup == nullptr)
				? ChildBuilder.AddProperty(ChildHandle.ToSharedRef())
				: DetailGroup->AddPropertyRow(ChildHandle.ToSharedRef());

			// If this group is driven by a Bool Header, disable child rows when bool is false
			if (GroupData->HeaderProperty.IsValid()) {
				TSharedPtr<IPropertyHandle> HeaderBoolHandle = GroupData->HeaderProperty;
				PropertyRow.IsEnabled(TAttribute<bool>::CreateLambda([HeaderBoolHandle]() -> bool {
					bool bValue = false;
					if (HeaderBoolHandle.IsValid() && HeaderBoolHandle->GetValue(bValue) == FPropertyAccess::Success) {
						return bValue;
					}
					return true;
				}));
			}

			const FName PropertyName = ChildHandle->GetProperty()->GetFName();

			// Special handling for specific names.
			if (PropertyName == FName("mMinVal") || PropertyName == FName("mMaxVal")) {
				CustomizeChild_MinMaxValue(PropertyHandle, ChildHandle, PropertyRow, CustomizationUtils);
			}
			else if (PropertyName == FName("MaxFractionalDigitsToDisplay")) {
				CustomizeChild_MaxFractionalDigitsToDisplay(PropertyHandle, ChildHandle, PropertyRow, CustomizationUtils);
			}
			else if (PropertyName == GET_MEMBER_NAME_CHECKED(FBBW_Slider_Data, ScaleType)) {
				CustomizeChild_ScaleType(PropertyHandle, ChildHandle, PropertyRow, CustomizationUtils);
			}
			else if (PropertyName == GET_MEMBER_NAME_CHECKED(FBBW_Slider_Data, SliderSnappingSize)) {
				CustomizeChild_SliderSnappingSize(PropertyHandle, ChildHandle, PropertyRow, CustomizationUtils);
			}
			else if (PropertyName == GET_MEMBER_NAME_CHECKED(FBBW_Slider_Data, LogarithmicBase)) {
				CustomizeChild_LogarithmicBase(PropertyHandle, ChildHandle, PropertyRow, CustomizationUtils);
			}
			else if (PropertyName == GET_MEMBER_NAME_CHECKED(FBBW_Slider_Data, MinSliderValueOverride) || PropertyName == GET_MEMBER_NAME_CHECKED(FBBW_Slider_Data, MaxSliderValueOverride)) {
				CustomizeChild_SliderValueOverride(PropertyHandle, ChildHandle, PropertyRow, CustomizationUtils);
			}
		}

		// Process subgroups
		for (auto& [SubName, SubGroup] : GroupData->Subgroups) {
			IDetailGroup& SubDetailGroup = (DetailGroup == nullptr)
				? ChildBuilder.AddGroup(FName(*SubName), FText::FromString(SubName), true)
				: DetailGroup->AddGroup(FName(*SubName), FText::FromString(SubName), true);

			if (SubGroup->HeaderProperty.IsValid()) {
				SubDetailGroup.HeaderRow()
				    .NameContent() [
						SubGroup->HeaderProperty->CreatePropertyNameWidget()
					]
					.ValueContent() [
						SubGroup->HeaderProperty->CreatePropertyValueWidget()
					];
			}

			GroupStack.Add(FGroupStackElement{ SubGroup, &SubDetailGroup });
		}
	}
}

EBBW_Slider_ScaleType FBBWE_Slider_Data_Customization::GetScaleType(TSharedPtr<IPropertyHandle> PropertyHandle) {
	TSharedRef<IPropertyHandle> ScaleTypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBBW_Slider_Data, ScaleType)).ToSharedRef();
	uint8 EnumValue = 0;
	EBBW_Slider_ScaleType ScaleType = EBBW_Slider_ScaleType::Linear;
	if (ScaleTypeHandle->GetValue(EnumValue) == FPropertyAccess::Success) {
		ScaleType = static_cast<EBBW_Slider_ScaleType>(EnumValue);
	}
	return ScaleType;
}

void FBBWE_Slider_Data_Customization::CustomizeChild_ScaleType(
	TSharedPtr<IPropertyHandle> PropertyHandle,
	TSharedPtr<IPropertyHandle> ChildHandle,
	IDetailPropertyRow& PropertyRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils
) {
	ChildHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateStatic(OnChanged_ScaleType, PropertyHandle, ChildHandle)
	);
}

void FBBWE_Slider_Data_Customization::CustomizeChild_MinMaxValue(
	TSharedPtr<IPropertyHandle> PropertyHandle,
	TSharedPtr<IPropertyHandle> ChildHandle,
	IDetailPropertyRow& PropertyRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils
) {
	const bool IsMin = ChildHandle->GetProperty()->GetFName() == FName("mMinVal");
	const FText LabelText = IsMin ? FText::FromString("Minimum Value") : FText::FromString("Maximum Value");
	const FText TooltipText = IsMin
		? FText::FromString("The minimum value the slider can be set to.")
		: FText::FromString("The maximum value the slider can be set to.");

	PropertyRow.CustomWidget()
		.NameContent()[
			SNew(STextBlock)
			.Text(LabelText)
			.ToolTipText(TooltipText)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()[
			ChildHandle->CreatePropertyValueWidget()
		];
}

void FBBWE_Slider_Data_Customization::CustomizeChild_MaxFractionalDigitsToDisplay(
	TSharedPtr<IPropertyHandle> PropertyHandle,
	TSharedPtr<IPropertyHandle> ChildHandle,
	IDetailPropertyRow& PropertyRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils
) {
	PropertyRow.CustomWidget()
		.NameContent()[
			SNew(STextBlock)
			.Text(FText::FromString("Max Fractional Digits to Display"))
			.ToolTipText(FText::FromString("No more than this many digits after the decimal point will be displayed."))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()[
			ChildHandle->CreatePropertyValueWidget()
		];
}

void FBBWE_Slider_Data_Customization::CustomizeChild_LogarithmicBase(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> ChildHandle, IDetailPropertyRow& PropertyRow, IPropertyTypeCustomizationUtils& CustomizationUtils) {
	ChildHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateStatic(OnChanged_LogarithmicBase, PropertyHandle, ChildHandle)
	);
}

void FBBWE_Slider_Data_Customization::CustomizeChild_SliderSnappingSize(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> ChildHandle, IDetailPropertyRow& PropertyRow, IPropertyTypeCustomizationUtils& CustomizationUtils) {
	PropertyRow
		.CustomWidget()
		.NameContent()[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text_Lambda([PropertyHandle]() -> FText {
				switch (GetScaleType(PropertyHandle)) {
					case EBBW_Slider_ScaleType::Linear:
						return FText::FromString("Slider Step Size");
					case EBBW_Slider_ScaleType::Logarithmic:
						return FText::FromString("Slider Exponential Step Size");
					default:
						checkNoEntry();
				}
			})
			.ToolTipText_Lambda([PropertyHandle]() -> FText {
				switch (GetScaleType(PropertyHandle)) {
					case EBBW_Slider_ScaleType::Linear:
						return FText::FromString("The step size for the slider, defining the interval between valid slider values.");
					case EBBW_Slider_ScaleType::Logarithmic:
						return FText::FromString("The step size for the slider, defining the interval between the exponent of the valid slider values.");
					default:
						checkNoEntry();
				}
			})
		]
		.ValueContent()[
			PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FBBW_Slider_Data, SliderSnappingSize))->CreatePropertyValueWidget()
		];

	ChildHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateStatic(OnChanged_LogarithmicBase, PropertyHandle, ChildHandle)
	);
}

void FBBWE_Slider_Data_Customization::CustomizeChild_SliderValueOverride(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> ChildHandle, IDetailPropertyRow& PropertyRow, IPropertyTypeCustomizationUtils& CustomizationUtils) {
	ChildHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateStatic(OnChanged_SliderValue, PropertyHandle, ChildHandle)
	);
}

void FBBWE_Slider_Data_Customization::OnChanged_ScaleType(
	TSharedPtr<IPropertyHandle> PropertyHandle,
	TSharedPtr<IPropertyHandle> ScaleTypeHandle
) {
	// uint8 EnumValue = 0;
	// if (ScaleTypeHandle->GetValue(EnumValue) == FPropertyAccess::Success) {
	// 	switch (static_cast<EBBW_Slider_ScaleType>(EnumValue)) {
	// 		case EBBW_Slider_ScaleType::Linear: {
	// 			break;
	// 		}
	// 		case EBBW_Slider_ScaleType::Logarithmic: {
	// 			break;
	// 		}
	// 		default: {
	// 			checkNoEntry();
	// 		}
	// 	}
	// }
}

void FBBWE_Slider_Data_Customization::OnChanged_LogarithmicBase(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> LogarithmicBaseHandle) {
	double CurrentValue;
	LogarithmicBaseHandle->GetValue(CurrentValue);

	if (CurrentValue <= 1.0) {
		LogarithmicBaseHandle->SetValue(2.0);
	}
}

void FBBWE_Slider_Data_Customization::OnChanged_SliderSnappingSize(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> SliderSnappingSizeHandle) {
	double CurrentValue;
	SliderSnappingSizeHandle->GetValue(CurrentValue);

	if (CurrentValue <= 0.0) {
		SliderSnappingSizeHandle->SetValue(1.0);
	}
}

void FBBWE_Slider_Data_Customization::OnChanged_SliderValue(TSharedPtr<IPropertyHandle> PropertyHandle, TSharedPtr<IPropertyHandle> SliderValueHandle) {
	switch (GetScaleType(PropertyHandle)) {
		case EBBW_Slider_ScaleType::Linear: {
			return;
		}
		case EBBW_Slider_ScaleType::Logarithmic: {
			double CurrentValue;
			SliderValueHandle->GetValue(CurrentValue);

			if (CurrentValue < 0.0) {
				SliderValueHandle->SetValue(0.0);
			}
			return;
		}
		default: {
			checkNoEntry();
		}
	}
}
