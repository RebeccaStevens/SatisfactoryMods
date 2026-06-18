#pragma once

#include "CoreMinimal.h"

#include "FGRecipeManager.h"

#include "Subsystem/ModSubsystem.h"
#include "UI/FGInteractWidget.h"
#include "UI/FGUserWidget.h"
#include "UI/FGWidgetSwitcher.h"
#include "UI/FGWindow.h"

#include "UserSettings/BGC_BuildMode_Data.h"

#include "BGC_AbstractPlayerSubsystem.generated.h"

UCLASS(Abstract)
class BUILDGUNCONFIG_API ABGC_AbstractPlayerSubsystem : public AModSubsystem {
  GENERATED_BODY()

protected:
  /**
   * The recipe manager.
   */
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = false, EditConditionHides))
  TObjectPtr<AFGRecipeManager> RecipeManager;

  /**
   * The navigation history of the widget switcher.
   */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BuildGunConfig", meta = (EditCondition = false, EditConditionHides))
  TArray<int32> NavigationHistory;

  /**
   * The build mode data for each build mode for each hologram that we have data on.
   */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BuildGunConfig|BuildModes")
  TMap<TSubclassOf<AFGHologram>, FBGC_BuildMode_Data> BuildModesData;

  /**
   * Any holograms that should **always** share the same build mode data as another hologram.
   */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BuildGunConfig|BuildModes")
  TMap<TSubclassOf<AFGHologram>, TSubclassOf<AFGHologram>> BuildModesAliases;

public:
  virtual void BeginPlay() override;

  /**
   * Retrieves the icon for a specific hologram.
   *
   * @param HologramClass The class of the hologram to retrieve the icon for. This must be a key in the BuildModesData
   * map (it cannot be an alias).
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  UTexture2D* GetBuildModeIconChecked(TSubclassOf<AFGHologram> HologramClass);

  /**
   * Retrieves the build mode data for every build mode of a specific hologram.
   *
   * @param HologramClass The class of the hologram to retrieve the data for.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  FBGC_BuildMode_Data& GetBuildModeDataChecked(TSubclassOf<AFGHologram> HologramClass);

  /**
   * Retrieves the build mode data for a specific build mode of a specific hologram.
   *
   * @param HologramClass The class of the hologram to retrieve the data for. This must be a key in the BuildModesData
   * map (it cannot be an alias).
   * @param BuildModeClass The class of the build mode to retrieve the data for.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  FBGC_BuildMode_DataEntry& GetBuildModeDataEntryChecked(TSubclassOf<AFGHologram> HologramClass, TSubclassOf<UFGBuildGunModeDescriptor> BuildModeClass);

  /**
   * Resets all build mode data.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  void ResetBuildModeData(bool bSave = true);

  /**
   * Removes all build mode data for a specific hologram.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  void RemoveBuildModeData(TSubclassOf<AFGHologram> HologramClass, bool bSave = true);

  /**
   * Removes the build mode data for a specific build mode of a specific hologram.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  void RemoveBuildModeDataEntry(TSubclassOf<AFGHologram> HologramClass, TSubclassOf<UFGBuildGunModeDescriptor> BuildModeClass, bool bSave = true);

  /**
   * Sets the build mode data of a specific hologram.
   *
   * @param HologramClass The class of the hologram the build mode belongs to. This must be a key in the BuildModesData
   * map (it cannot be an alias).
   * @param HologramBuildModeData The build mode data to save.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  void SetBuildModeDataChecked(TSubclassOf<AFGHologram> HologramClass, UPARAM(ref) const FBGC_BuildMode_Data& HologramBuildModeData, bool bSave = true);

  /**
   * Set the build mode data for a specific build mode of a specific hologram.
   *
   * @param HologramClass The class of the hologram the build mode belongs to. This must be a key in the BuildModesData
   * map (it cannot be an alias).
   * @param BuildModeClass The class of the build mode to save the data for.
   * @param HologramBuildModeDataEntry The build mode data to save.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  void SetBuildModeDataEntryChecked(
      TSubclassOf<AFGHologram> HologramClass,
      TSubclassOf<UFGBuildGunModeDescriptor> BuildModeClass,
      UPARAM(ref) const FBGC_BuildMode_DataEntry& HologramBuildModeDataEntry,
      bool bSave = true);

  /**
   * Should configurations for buildables that are locked be shown?
   */
  UFUNCTION(BlueprintImplementableEvent, Category = "BuildGunConfig|BuildModes")
  bool ShowLockedBuildables();

  /**
   * Rebuilds the build mode data.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  void RebuildBuildModesData();

  /**
   * Find the build mode data for a hologram class, following any inheritance.
   * If there is no inheritance, the original data is returned.
   *
   * @param HologramClass The hologram class to search for.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  const FBGC_BuildMode_Data& ResolveBuildModeInheritance(TSubclassOf<AFGHologram> HologramClass, UPARAM(ref) const FBGC_BuildMode_Data& HologramBuildModeData);

  /**
   * Retrieves the build mode data for a specific hologram, following aliases and inheritance.
   *
   * @param HologramClass The hologram class to search for.
   * @param out_BuildModeData The build mode data to fill.
   * @return true if the build mode data was found, false otherwise.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  bool ResolveBuildModeData(TSubclassOf<AFGHologram> HologramClass, UPARAM(DisplayName = "ResolvedBuildModeData") FBGC_BuildMode_Data& out_BuildModeData);

  /**
   * Serializes the build mode data.
   */
  FString BuildModesDataToJsonString();

  /**
   * Get a JSON representation of the mutable parts of the build mode data.
   */
  TSharedPtr<FJsonObject> BuildModesDataToJson();

  /**
   * Saves the build mode data to the session settings.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  void SaveBuildModeData();

  /**
   * Loads the build mode data from the session settings.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  void LoadBuildModeData();

  /**
   * Loads the build mode data from a JSON string.
   *
   * @param JsonString The JSON string to load the build mode data from.
   */
  void LoadBuildModeData(const FString& JsonString);

  /**
   * Loads the build mode data from a JSON object.
   *
   * @param JsonObject The JSON object to load the build mode data from.
   */
  void LoadBuildModeData(TSharedPtr<FJsonObject> JsonObject);

  /**
   * Filter and sort the build modes for a hologram.
   *
   * @param out_BuildModes The build modes modified in place.
   * @param HologramData The hologram data to use.
   */
  UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
  void
  FilterAndSortBuildModes(UPARAM(ref) const FBGC_BuildMode_Data& HologramBuildModeData, TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& out_BuildModes) const;

private:
#if WITH_EDITOR
  void SortAndReindexBuildModesData();
  virtual EDataValidationResult ValidateBuildModeInheritance(FDataValidationContext& Context) const;
  virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
