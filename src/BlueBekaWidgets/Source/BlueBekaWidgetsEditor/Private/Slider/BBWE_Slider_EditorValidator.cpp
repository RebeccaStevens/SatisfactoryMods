#include "Slider/BBWE_Slider_EditorValidator.h"

#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"

#include "BBWE_Module.h"

TArray<const UWidget*> UBBWE_Slider_EditorValidator::GetSliderChildren(
	const UWidgetBlueprint* BlueprintWidget
) const {
	TArray<const UWidget*> Children = BlueprintWidget->GetAllSourceWidgets().FilterByPredicate([&](const UWidget* ChildWidget) {
		return ChildWidget->IsA(SliderClass);
	});
	return Children;
}

void UBBWE_Slider_EditorValidator::FailAsset(UObject* Asset, const FText Message) {
	AssetFails(Asset, Message);
}

bool UBBWE_Slider_EditorValidator::CanValidateAsset_Implementation(
	const FAssetData& AssetData,
	UObject* Object,
	FDataValidationContext& Context
) const {
	return IsValid(Cast<const UWidgetBlueprint>(Object)) && !GetSliderChildren(Cast<const UWidgetBlueprint>(Object)).IsEmpty();
}

EDataValidationResult UBBWE_Slider_EditorValidator::ValidateLoadedAsset_Implementation(
	const FAssetData& AssetData,
	UObject* Asset,
	FDataValidationContext& Context
) {
	const auto BlueprintWidget = CastChecked<UWidgetBlueprint>(Asset);

	EDataValidationResult Result = EDataValidationResult::NotValidated;
	TArray<const UWidget*> Children = GetSliderChildren(BlueprintWidget);
	for (const auto Child : Children) {
		const auto ChildResult = ValidateSlider(AssetData, Child);
		switch (Result) {
			case EDataValidationResult::Invalid:
				break;
			default:
				Result = ChildResult;
				break;
		}
	}

	if (Result == EDataValidationResult::Valid) {
		AssetPasses(Asset);
	}
	return Result;
}
