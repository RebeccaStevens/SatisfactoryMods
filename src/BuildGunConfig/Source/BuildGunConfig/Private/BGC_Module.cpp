#include "BGC_Module.h"

#define LOCTEXT_NAMESPACE "FBuildGunConfigModule"

DEFINE_LOG_CATEGORY(LogBuildGunConfig);

void FBuildGunConfigModule::StartupModule() {}

void FBuildGunConfigModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBuildGunConfigModule, BuildGunConfig)
