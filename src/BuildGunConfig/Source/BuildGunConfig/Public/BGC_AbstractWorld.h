#pragma once

#include "CoreMinimal.h"

#include "Module/GameWorldModule.h"

#include "Config/BGC_BuildMode_Data.h"

#include "BGC_AbstractWorld.generated.h"

/**
 * The C++ code for the root game world module of this mod.
 */
UCLASS(Abstract)
class BUILDGUNCONFIG_API UBGC_AbstractWorld : public UGameWorldModule {
  GENERATED_BODY()

public:
  /**
   * Returns the root game world module of this mod.
   */
  UFUNCTION(
    BlueprintCallable,
    BlueprintPure,
    Category = "BuildGunConfig",
    meta = (DisplayName = "Get BGC Root World", CompactNodeTitle = "BGC Root World", DefaultToSelf = "WorldContext"))
  static UBGC_AbstractWorld* Get(const UObject* WorldContext);

  virtual void DispatchLifecycleEvent(ELifecyclePhase Phase) override;

  /**
   * Set up our hooks.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig")
  void SubscribeToHooks();

protected:
  /**
   * The player subsystem where data for this client is stored.
   */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BuildGunConfig")
  ABGC_AbstractPlayerSubsystem* PlayerSubsystem;

private:
};
