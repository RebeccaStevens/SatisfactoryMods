#pragma once

#include <Dom/JsonObject.h>

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "Templates/SubclassOf.h"

#include "FGBuildGunModeDescriptor.h"
#include "Hologram/FGHologram.h"
#include "Resources/FGBuildingDescriptor.h"

#include "BGC_Module.h"

#include "BGC_BuildModeGroup.generated.h"

USTRUCT(BlueprintType, Category = "BuildGunConfig|BuildModes")
struct FBGC_BuildModeGroup_BuildMode {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuildGunConfig|BuildModes")
	int32 Weight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuildGunConfig|BuildModes")
	bool IsEnabled = true;

	/**
	 * Get a JSON representation of the user configurable data of this build mode data entry.
	 */
	TSharedPtr<FJsonObject> ToJson() const {
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
		JsonObject->SetNumberField(TEXT("Weight"), Weight);
		JsonObject->SetBoolField(TEXT("IsEnabled"), IsEnabled);
		return JsonObject;
	}

	/**
	 * Reset the data to its default values.
	 */
	void Reset() {
		Weight = 0;
		IsEnabled = true;
	}

	/**
	 * Create an instance of this build mode data entry from a JSON object.
	 */
	static FBGC_BuildModeGroup_BuildMode FromJson(const TSharedPtr<FJsonObject>& JsonObject) {
		FBGC_BuildModeGroup_BuildMode BuildModeEntry;
		BuildModeEntry.Weight = JsonObject->GetNumberField(TEXT("Weight"));
		BuildModeEntry.IsEnabled = JsonObject->GetBoolField(TEXT("IsEnabled"));
		return BuildModeEntry;
	}

	/**
	 * Load the build mode JSON data into the given build mode.
	 */
	static void LoadFromJson(const TSharedRef<FJsonObject> JsonObject, FBGC_BuildModeGroup_BuildMode* BuildMode) {
		if (!JsonObject->TryGetNumberField(TEXT("Weight"), BuildMode->Weight)) {
			BuildMode->Weight = INDEX_NONE;
			UE_LOG(LogBuildGunConfig, Error, TEXT("Failed to get build mode weight."));
			return;
		}
		if (!JsonObject->TryGetBoolField(TEXT("IsEnabled"), BuildMode->IsEnabled)) {
			BuildMode->IsEnabled = true;
		}
	}
};

USTRUCT(BlueprintType, BlueprintInternalUseOnly, Category = "BuildGunConfig|BuildModes")
struct FBGC_BuildModeGroupCommon {
	GENERATED_BODY()

	/**
	 * The name displayed to the player for this build modes group.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildGunConfig|BuildModes")
	FText DisplayName;

	/**
	 * The icon displayed to the player for this build modes group.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "BuildGunConfig|BuildModes",
		meta=(
			DisplayThumbnail = "true",
			AllowedClasses = "/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface",
			DisallowedClasses = "/Script/MediaAssets.MediaTexture")
	)
	TObjectPtr<UObject> Icon;

	/**
	 * Whether this configuration is enabled.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildGunConfig|BuildModes")
	bool IsEnabled = true;

	/**
	 * The index of the build mode group that build modes can be inherited from (-1 = disabled).
	 */
	UPROPERTY(EditDefaultsOnly, Category = "BuildGunConfig|BuildModes")
	int32 InheritBuildModesFrom = INDEX_NONE;
};

USTRUCT(BlueprintType, Category = "BuildGunConfig|BuildModes")
struct FBGC_BuildModeGroup : public FBGC_BuildModeGroupCommon {
	GENERATED_BODY()

	/**
	 * An identifier for this group to distinguish it form others.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "BuildGunConfig|BuildModes")
	int32 Id = INDEX_NONE;

	/**
	 * The list of building descriptors that this configuration applies to.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "BuildGunConfig|BuildModes")
	TArray<TSubclassOf<UFGBuildingDescriptor>> AppliesTo;

	/**
	 * The build mode data for how each build mode should be modified.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "BuildGunConfig|BuildModes")
	TMap<TSubclassOf<UFGBuildGunModeDescriptor>, FBGC_BuildModeGroup_BuildMode> BuildModes;

	/**
	 * If set to true, the build modes for the specified class in InheritBuildModesFrom will be used instead of the ones
	 * specified in BuildModes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuildGunConfig|BuildModes")
	bool InheritBuildModes = true;

	/**
	 * Reset the data to its default values.
	 */
	void Reset() {
		InheritBuildModes = true;
		int BuildModeIndex = 0;
		for (auto& [_, Data] : BuildModes) {
			Data.Reset();
			Data.Weight = BuildModeIndex++;
		}
	}

	/**
	 * Get a JSON representation of the user configurable data of this build mode data.
	 */
	TSharedPtr<FJsonObject> ToJson() const {
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

		JsonObject->SetNumberField(TEXT("Id"), Id);

		if (InheritBuildModesFrom != INDEX_NONE) {
			JsonObject->SetBoolField(TEXT("InheritBuildModes"), InheritBuildModes);
		}

		if (BuildModes.Num() > 0) {
			const TSharedPtr<FJsonObject> BuildModesObject = MakeShareable(new FJsonObject());
			for (const auto& BuildModeEntry : BuildModes) {
				BuildModesObject->SetObjectField(BuildModeEntry.Key->GetPathName(), BuildModeEntry.Value.ToJson());
			}
			JsonObject->SetObjectField(TEXT("BuildModes"), BuildModesObject);
		}

		return JsonObject;
	}

	/**
	 * Load the build mode group JSON data into the given build mode groups.
	 */
	static void LoadFromJson(
		const TSharedRef<FJsonObject> JsonObject,
		TArray<TSharedRef<FBGC_BuildModeGroup>>& BuildModeGroups
	) {
		int32 Id;
		if (!JsonObject->TryGetNumberField(TEXT("Id"), Id)) {
			UE_LOG(LogBuildGunConfig, Warning, TEXT("No build mode group id"));
			return;
		}
		if (!BuildModeGroups.IsValidIndex(Id)) {
			UE_LOG(LogBuildGunConfig, Warning, TEXT("Invalid build mode group id  \"%i\"... skipping."), Id);
			return;
		}

		const auto BuildModeGroup = BuildModeGroups[Id];
		if (!JsonObject->TryGetBoolField(TEXT("InheritBuildModes"), BuildModeGroup->InheritBuildModes)) {
			BuildModeGroup->InheritBuildModes = false;
		}

		if (const TSharedPtr<FJsonObject>* SavedBuildModes;
			JsonObject->TryGetObjectField(TEXT("BuildModes"), SavedBuildModes)) {
			for (const auto& SavedBuildMode : (*SavedBuildModes)->Values) {
				const auto SavedBuildModeClass = LoadClass<UFGBuildGunModeDescriptor>(nullptr, *SavedBuildMode.Key);
				if (SavedBuildModeClass == nullptr) {
					UE_LOG(
						LogBuildGunConfig,
						Warning,
						TEXT("Failed to load build mode class from %s"),
						*SavedBuildMode.Key
					);
					continue;
				}

				const auto BuildMode = BuildModeGroup->BuildModes.Find(SavedBuildModeClass);
				if (BuildMode == nullptr) {
					UE_LOG(
						LogBuildGunConfig,
						Warning,
						TEXT("Unsupported build mode class \"%s\"... skipping."),
						*SavedBuildModeClass->GetName()
					);
					continue;
				}

				auto SavedBuildModeDataObject = SavedBuildMode.Value->AsObject();
				if (!SavedBuildModeDataObject.IsValid()) {
					continue;
				}

				FBGC_BuildModeGroup_BuildMode::LoadFromJson(SavedBuildModeDataObject.ToSharedRef(), BuildMode);
			}
		}
	}
};

USTRUCT(BlueprintType, Category = "BuildGunConfig|BuildModes")
struct FBGC_PredefinedBuildModeGroup : public FBGC_BuildModeGroupCommon {
	GENERATED_BODY()

	/**
	 * The holograms that should be included in this group.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "BuildGunConfig|BuildModes")
	TSet<TSubclassOf<AFGHologram>> Holograms;

	TSharedRef<FBGC_BuildModeGroup> CreateBuildModeGroup() const {
		auto BuildModeGroup = MakeShared<FBGC_BuildModeGroup>();
		BuildModeGroup->DisplayName = DisplayName;
		BuildModeGroup->Icon = Icon;
		BuildModeGroup->IsEnabled = IsEnabled;
		BuildModeGroup->InheritBuildModesFrom = InheritBuildModesFrom;
		return BuildModeGroup;
	}
};
