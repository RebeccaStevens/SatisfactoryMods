#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Templates/SubclassOf.h"

#include "FGBuildGunModeDescriptor.h"
#include "Hologram/FGHologram.h"
#include "Resources/FGBuildingDescriptor.h"

#include "BGC_Module.h"

#include "BGC_BuildMode_Data.generated.h"

USTRUCT(BlueprintType, Category = "BuildGunConfig|BuildModes")
struct FBGC_BuildMode_DataEntry {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuildGunConfig|BuildModesData")
  int32 Index = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuildGunConfig|BuildModesData")
  bool IsEnabled = true;

  /**
   * Get a JSON representation of the user configurable data of this build mode data entry.
   */
  TSharedPtr<FJsonObject> ToJson() const {
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetNumberField(TEXT("Index"), Index);
    JsonObject->SetBoolField(TEXT("IsEnabled"), IsEnabled);
    return JsonObject;
  }

  /**
   * Create an instance of this build mode data entry from a JSON object.
   */
  static FBGC_BuildMode_DataEntry FromJson(const TSharedPtr<FJsonObject> JsonObject) {
    FBGC_BuildMode_DataEntry buildModeEntry;
    buildModeEntry.Index = JsonObject->GetNumberField(TEXT("Index"));
    buildModeEntry.IsEnabled = JsonObject->GetBoolField(TEXT("IsEnabled"));
    return buildModeEntry;
  }
};

USTRUCT(BlueprintType, Category = "BuildGunConfig|BuildModes")
struct FBGC_BuildMode_Data {
  GENERATED_BODY()

#if WITH_EDITORONLY_DATA
  UPROPERTY(
    EditAnywhere,
    meta =
      (DisplayPriority = 0,
       DisplayName = "Weight",
       ToolTip = "Used to sort this value in the map. Auto-reset on map sort."))
  float EditorWeight = 0.f;
#endif

  /**
   * The name displayed to the player for this group build modes.
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuildGunConfig|BuildModesData")
  FText DisplayName;

  /**
   * The icon displayed to the player for this group build modes.
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuildGunConfig|BuildModesData")
  TObjectPtr<UTexture2D> Icon;

  /**
   * Whether or not this configuration is enabled.
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuildGunConfig|BuildModesData")
  bool IsEnabled = true;

  /**
   * The list of building descriptors that this configuration applies to.
   */
  UPROPERTY(
    EditAnywhere,
    BlueprintReadWrite,
    Category = "BuildGunConfig|BuildModesData",
    meta = (EditCondition = "false", EditConditionHides))
  TArray<TSubclassOf<UFGBuildingDescriptor>> AppliesTo;

  /**
   * The build mode data for how each build mode should be modified.
   */
  UPROPERTY(
    EditAnywhere,
    BlueprintReadWrite,
    Category = "BuildGunConfig|BuildModesData",
    meta = (EditCondition = "false", EditConditionHides))
  TMap<TSubclassOf<UFGBuildGunModeDescriptor>, FBGC_BuildMode_DataEntry> BuildModes;

  /**
   * If set to true, the build modes for the specified class in InheritBuildModesFrom will be used instead of the ones
   * specified in BuildModes.
   */
  UPROPERTY(
    EditAnywhere,
    BlueprintReadWrite,
    Category = "BuildGunConfig|BuildModesData",
    meta = (EditCondition = "false", EditConditionHides))
  bool InheritBuildModes = false;

  /**
   * If set along with InheritBuildModes, the build modes for the specified class will be used instead of the ones
   * specified in BuildModes.
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuildGunConfig|BuildModesData")
  TSubclassOf<AFGHologram> InheritBuildModesFrom;

  /**
   * Get a JSON representation of the user configurable data of this build mode data.
   */
  TSharedPtr<FJsonObject> ToJson() const {
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

    if (InheritBuildModesFrom != nullptr) {
      JsonObject->SetBoolField(TEXT("InheritBuildModes"), InheritBuildModes);
    }

    if (BuildModes.Num() > 0) {
      TSharedPtr<FJsonObject> BuildModesObject = MakeShareable(new FJsonObject());
      for (const auto& BuildModeEntry : BuildModes) {
        BuildModesObject->SetObjectField(BuildModeEntry.Key->GetPathName(), BuildModeEntry.Value.ToJson());
      }
      JsonObject->SetObjectField(TEXT("BuildModes"), BuildModesObject);
    }

    return JsonObject;
  }

  /**
   * Create an instance of this build mode data from a JSON object.
   */
  static FBGC_BuildMode_Data FromJson(const TSharedPtr<FJsonObject> JsonObject) {
    FBGC_BuildMode_Data buildModeData;
    buildModeData.InheritBuildModes = JsonObject->GetBoolField(TEXT("InheritBuildModes"));

    TSharedPtr<FJsonObject> buildModesObject = JsonObject->GetObjectField(TEXT("BuildModes"));
    for (const auto& buildMode : buildModesObject->Values) {
      auto buildModeClass = LoadClass<UFGBuildGunModeDescriptor>(nullptr, *buildMode.Key);
      if (buildModeClass == nullptr) {
        UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to load build mode class from %s"), *buildMode.Key);
        continue;
      }
      auto buildModeDataObject = buildMode.Value->AsObject();
      if (buildModeDataObject == nullptr) {
        continue;
      }
      auto buildModeEntry = FBGC_BuildMode_DataEntry::FromJson(buildModeDataObject);
      buildModeData.BuildModes.Add(buildModeClass, buildModeEntry);
    }

    return buildModeData;
  }

  void Reset() {
    InheritBuildModes = false;
    BuildModes.Empty();
  }

  /**
   * Returns true if the user configurable data of this build mode data is the default build mode data.
   */
  bool IsDefault() const {
    if (InheritBuildModes == true) {
      return false;
    }
    auto index = 0;
    for (auto buildMode : BuildModes) {
      if (buildMode.Value.Index != index) {
        return false;
      }
      if (!buildMode.Value.IsEnabled) {
        return false;
      }
      index++;
    }
    return true;
  }
};
