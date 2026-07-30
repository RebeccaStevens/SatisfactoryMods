#include "BBWE_Module.h"

#include "PropertyEditorModule.h"

#include "Slider/BBWE_Slider_Data_Customization.h"
#include "Slider/BBW_Slider_Data.h"

#define LOCTEXT_NAMESPACE "FBlueBekaWidgetsEditorModule"

DEFINE_LOG_CATEGORY(LogBBWE);

void FBlueBekaWidgetsEditorModule::StartupModule() {
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomPropertyTypeLayout(
		FBBW_Slider_Data::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FBBWE_Slider_Data_Customization::MakeInstance)
	);
}

void FBlueBekaWidgetsEditorModule::ShutdownModule() {
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor")) {
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout(FBBW_Slider_Data::StaticStruct()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBlueBekaWidgetsEditorModule, BlueBekaWidgetsEditor)
