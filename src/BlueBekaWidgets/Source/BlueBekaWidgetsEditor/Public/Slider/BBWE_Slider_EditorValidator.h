#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "WidgetBlueprint.h"
#include "Blueprint/UserWidget.h"

#include "BBWE_Slider_EditorValidator.generated.h"

UCLASS(Abstract)
class BLUEBEKAWIDGETSEDITOR_API UBBWE_Slider_EditorValidator : public UEditorValidatorBase {
	GENERATED_BODY()

public:
	virtual bool IsEditorOnly() const override { return true; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<UUserWidget> SliderClass;

	/**
	 * Get all the source widgets of the given widget that are a SliderClass.
	 */
	TArray<const UWidget*> GetSliderChildren(const UWidgetBlueprint* BlueprintWidget) const;

	/**
	 * Fail the Asset.
	 */
	UFUNCTION(BlueprintCallable)
	void FailAsset(UObject* Asset, const FText Message);

	UFUNCTION(BlueprintImplementableEvent)
	EDataValidationResult ValidateSlider(const FAssetData& AssetData, const UObject* Asset);

	virtual bool CanValidateAsset_Implementation(const FAssetData& AssetData, UObject* Object, FDataValidationContext& Context) const override;

	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& AssetData, UObject* Asset, FDataValidationContext& Context) override;
};
