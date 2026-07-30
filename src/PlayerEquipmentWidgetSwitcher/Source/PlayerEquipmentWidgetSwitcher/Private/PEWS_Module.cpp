#include "PEWS_Module.h"

#define LOCTEXT_NAMESPACE "FPlayerEquipmentWidgetSwitcherModule"

DEFINE_LOG_CATEGORY(LogPEWS);

void FPlayerEquipmentWidgetSwitcherModule::StartupModule() {}

void FPlayerEquipmentWidgetSwitcherModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPlayerEquipmentWidgetSwitcherModule, PlayerEquipmentWidgetSwitcher)
