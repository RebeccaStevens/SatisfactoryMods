#pragma once

#include "CoreMinimal.h"

#include "Module/GameInstanceModule.h"

#include "BGC_AbstractInstance.generated.h"

/**
 * The C++ code for the root game instance module of this mod.
 */
UCLASS(Abstract)
class BUILDGUNCONFIG_API UBGC_AbstractInstance : public UGameInstanceModule {
  GENERATED_BODY()

public:
  /**
   * The mod name.
   */
  static const inline FName ModName = TEXT("BuildGunConfig");

  /**
   * Returns the mod name.
   */
  UFUNCTION(
    BlueprintCallable,
    BlueprintPure,
    Category = "BuildGunConfig",
    meta = (DisplayName = "Get BGC Mod Name", CompactNodeTitle = "BGC Mod Name"))

  static FName GetModName() {
    return ModName;
  }

  /**
   * Returns the root game instance module of this mod.
   */
  UFUNCTION(
    BlueprintCallable,
    BlueprintPure,
    Category = "BuildGunConfig",
    meta =
      (DisplayName = "Get BGC Root Instance", CompactNodeTitle = "BGC Root Instance", DefaultToSelf = "WorldContext"))
  static UBGC_AbstractInstance* Get(UObject* WorldContext);

  virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;

private:
};
