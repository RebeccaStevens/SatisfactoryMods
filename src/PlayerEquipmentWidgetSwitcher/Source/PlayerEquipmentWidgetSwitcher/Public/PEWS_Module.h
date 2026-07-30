#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPEWS, Verbose, All);

class FPlayerEquipmentWidgetSwitcherModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
