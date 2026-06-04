#include "BGC_AbstractWorld.h"

#include "CoreMinimal.h"

#include "Equipment/FGBuildGunBuild.h"
#include "FGBuildGunModeDescriptor.h"

#include "Patching/NativeHookManager.h"
#include "Subsystem/SubsystemActorManager.h"

#include "BGC_Module.h"
#include "Config/BGC_BuildMode_Data.h"

UBGC_AbstractWorld* UBGC_AbstractWorld::Get(const UObject* WorldContext) {
  if (!ensureMsgf(IsValid(WorldContext), TEXT("Invalid world context."))) {
    return nullptr;
  }

  auto World = WorldContext->GetWorld();
  if (!ensureMsgf(IsValid(World), TEXT("Failed to get the world."))) {
    return nullptr;
  }

  auto WorldModuleManager = World->GetSubsystem<UWorldModuleManager>();
  if (!ensureMsgf(IsValid(WorldModuleManager), TEXT("Failed to find world module manager."))) {
    return nullptr;
  }

  auto WorldModule = WorldModuleManager->FindModule(UBGC_AbstractInstance::ModName);
  if (!ensureMsgf(IsValid(WorldModule), TEXT("Failed to find world module."))) {
    return nullptr;
  }

  return Cast<UBGC_AbstractWorld>(WorldModule);
}

void UBGC_AbstractWorld::DispatchLifecycleEvent(ELifecyclePhase Phase) {
  Super::DispatchLifecycleEvent(Phase);
  switch (Phase) {
    case ELifecyclePhase::CONSTRUCTION: {
      return;
    }
    case ELifecyclePhase::INITIALIZATION: {
      return;
    }
    case ELifecyclePhase::POST_INITIALIZATION: {
      SubscribeToHooks();
      return;
    }
  }
}

void UBGC_AbstractWorld::SubscribeToHooks() {
  auto buildGunStateBuild = GetMutableDefault<UFGBuildGunStateBuild>();

  /**
   * Filter out disabled build modes and sort the rest according to our config data.
   */
  SUBSCRIBE_METHOD_VIRTUAL_AFTER(
    UFGBuildGunStateBuild::GetSupportedBuildModes_Implementation,
    buildGunStateBuild,
    [this](const UFGBuildGunStateBuild* self, TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& out_buildModes) {
      if (!IsValid(PlayerSubsystem) || out_buildModes.Num() <= 1) {
        return;
      }

      auto hologramClass = UBGC_Utils::GetHologramClass(self);
      if (!IsValid(hologramClass)) {
        return;
      }

      FBGC_BuildMode_Data resolvedHologramBuildModeData;
      if (!PlayerSubsystem->ResolveBuildModeData(hologramClass, resolvedHologramBuildModeData)) {
        return;
      }
      if (!resolvedHologramBuildModeData.IsEnabled) {
        return;
      }

      PlayerSubsystem->FilterAndSortBuildModes(resolvedHologramBuildModeData, out_buildModes);
    });

  /**
   * Use the first build mode as the initial build mode.
   */
  SUBSCRIBE_METHOD_VIRTUAL(
    UFGBuildGunStateBuild::GetInitialBuildGunMode_Implementation,
    buildGunStateBuild,
    [this](auto& scope, const UFGBuildGunStateBuild* self) {
      TSubclassOf<UFGBuildGunModeDescriptor> result = scope(self);
      TArray<TSubclassOf<UFGBuildGunModeDescriptor>> supportedBuildModes;
      self->GetSupportedBuildModes_Implementation(supportedBuildModes);
      if (supportedBuildModes.Num() > 0) {
        scope.Override(supportedBuildModes[0]);
      }
    });
}
