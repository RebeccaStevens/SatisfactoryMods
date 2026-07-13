#pragma once

#include "CoreMinimal.h"

#include "Subsystem/ModSubsystem.h"

#include "BGC_NavigationHistory.h"
#include "UserSettings/BGC_BuildModeGroup.h"

#include "BGC_AbstractPlayerSubsystem.generated.h"

UCLASS(Abstract)
class BUILDGUNCONFIG_API ABGC_AbstractPlayerSubsystem : public AModSubsystem {
	GENERATED_BODY()

protected:
	/**
	 * The navigation history of the widget switcher.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadWrite,
		Category = "BuildGunConfig",
		meta = (EditCondition = false, EditConditionHides)
	)
	TArray<FBGC_NavigationHistoryEntry> NavigationHistory;

	TArray<TSharedRef<FBGC_BuildModeGroup>> BuildModeGroups;
	TMap<TSubclassOf<AFGHologram>, TWeakPtr<FBGC_BuildModeGroup>> BuildModeGroupsByHologram;

	/**
	 * Known Build Mode Groups.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadWrite,
		Category = "BuildGunConfig|BuildModes",
		DisplayName = "Build Mode Groups"
	)
	TArray<FBGC_PredefinedBuildModeGroup> PredefinedBuildModeGroups;

	/**
	 * The display name used when a build mode has no display name.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "BuildGunConfig|BuildModes")
	FText DefaultBuildModeDisplayName;

	/**
	 * The icon used when a build mode has no icon.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "BuildGunConfig|BuildModes")
	TObjectPtr<UTexture2D> DefaultBuildModeIcon;

	virtual void BeginPlay() override;

public:
	/**
	 * Get the build mode group ids that should be shown in the UI.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	TArray<int32> GetBuildModeGroupIdsForDisplay() const;

	/**
	 * Get the build mode group with the specified id.
	 *
	 * @param BuildModeGroupId The id of the build mode group to get.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	FBGC_BuildModeGroup& GetBuildModeGroup(const int32 BuildModeGroupId);

	/**
	 * Get the supported build modes for a build mode group.
	 *
	 * @param BuildModeGroupId The id of the build mode group to get the build modes for.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	TArray<TSubclassOf<UFGBuildGunModeDescriptor>> GetSupportedBuildModes(const int32 BuildModeGroupId) const;

	/**
	 * Make sure the build mode group's data for the given id is valid.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	void EnsureValidBuildModeGroup(const int32 BuildModeGroupId);

	/**
	 * Make sure the given build mode group's data is valid.
	 */
	void EnsureValidBuildModeGroup(const TSharedRef<FBGC_BuildModeGroup> BuildModeGroup) const;

	/**
	 * Make sure the build mode group's build modes are valid for the given build mode group id.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	void EnsureValidBuildModeGroupBuildModes(const int32 BuildModeGroupId);

	/**
	 * Make sure the given build mode group's build modes are valid.
	 */
	static void EnsureValidBuildModeGroupBuildModes(const TSharedRef<FBGC_BuildModeGroup> BuildModeGroup);

	/**
	 * Resets all build mode groups.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	void ResetBuildModeData();

	/**
	 * Removes all build mode data for a specific build mode group.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	void RemoveBuildModeGroupData(const int32 BuildModeGroupId);

	/**
	 * Removes all build mode data for a specific build mode group.
	 */
	static void RemoveBuildModeGroupData(const TSharedRef<FBGC_BuildModeGroup> BuildModeGroup);

	/**
	 * Sets if the build mode group should inherit build modes from another build mode group.
	 *
	 * @param BuildModeGroupId The id of the build mode group to set should inherit for.
	 * @param ShouldInherit
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	void SetBuildModeGroupInherits(const int32 BuildModeGroupId, const bool ShouldInherit);

	/**
	 * Set the build mode data for a specific build mode within the specified build mode group.
	 *
	 * @param BuildModeGroupId The build mode group id to set the build mode data for.
	 * @param BuildModeClass The build mode class the data is for.
	 * @param BuildModeData The build mode data to add to the build mode group.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	void SetBuildModeGroupBuildMode(
		const int32 BuildModeGroupId,
		const TSubclassOf<UFGBuildGunModeDescriptor> BuildModeClass,
		UPARAM(ref) const FBGC_BuildModeGroup_BuildMode& BuildModeData
	);

	/**
	 * Should configurations for buildables that are not yet unlocked be shown?
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "BuildGunConfig|BuildModes")
	bool ShouldShowLockedBuildables() const;
	bool ShouldShowLockedBuildables_Implementation() const {
		checkNoEntry();
		return false;
	}

	/**
	 * Get the configured default nudge distance.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "BuildGunConfig|Nudging")
	float GetDefaultNudgeDistance() const;
	float GetDefaultNudgeDistance_Implementation() const {
		checkNoEntry();
		return 100.0f;
	}

	/**
	 * Get the configured default rotation step.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "BuildGunConfig|RotationStep")
	int32 GetDefaultRotationStep() const;
	int32 GetDefaultRotationStep_Implementation() const {
		checkNoEntry();
		return 15;
	}

	/**
	 * Rebuilds the build mode groups.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	void RebuildBuildModeGroups();

private:
	/**
	 * Create build mode groups from predefined group data.
	 * All existing build mode group data will be removed.
	 */
	void InitializeBuildModeGroups();

	/**
	 * Get a map of each building descriptor by the hologram that creates it.
	 */
	TSharedRef<TMap<TSubclassOf<AFGHologram>, TArray<TSubclassOf<UFGBuildingDescriptor>>>>
	GetBuildingDescriptorsByHologram();

	/**
	 * Group each hologram by the build mode list they support.
	 */
	static TSharedRef<TMap<TArray<TSubclassOf<UFGBuildGunModeDescriptor>>, TSet<TSubclassOf<AFGHologram>>>>
	GetHologramsByBuildModes(
		TSharedRef<TMap<TSubclassOf<AFGHologram>, TArray<TSubclassOf<UFGBuildingDescriptor>>>>
		BuildingDescriptorsByHologram
	);

	/**
	 * The return type data for `PopulateBuildModeGroups`.
	 */
	struct FBGC_BuildModeGroupData {
		TSet<TSubclassOf<AFGHologram>> Holograms;
		TArray<TSubclassOf<UFGBuildGunModeDescriptor>> BuildModes;
	};

	/**
	 * Add new build mode groups for holograms that don't already have a group.
	 */
	TMap<TWeakPtr<FBGC_BuildModeGroup>, FBGC_BuildModeGroupData> PopulateBuildModeGroups(
		TSharedRef<TMap<TArray<TSubclassOf<UFGBuildGunModeDescriptor>>, TSet<TSubclassOf<AFGHologram>>>> HologramsByBuildModes
	);

public:
	/**
	 * Find the id of the build mode group that the specified build mode group inherits from.
	 *
	 * @param BuildModeGroupId The id of the build mode group to resolve.
	 * @return The id of the build mode group to inherit from, or -1 if it doesn't inherit.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	int32 GetBuildModeGroupInheritFrom(const int32 BuildModeGroupId) const;

	/**
	 * Find the build mode group that the specified build mode group inherits from.
	 *
	 * @param BuildModeGroup The build mode group to resolve.
	 */
	TSharedPtr<FBGC_BuildModeGroup> GetBuildModeGroupInheritFrom(
		const TSharedRef<FBGC_BuildModeGroup> BuildModeGroup
	) const;

	/**
	 * Find the id of the build mode group at the top of the inheritance chain.
	 *
	 * @param BuildModeGroupId The id of the build mode group to resolve.
	 * @return The id of the build mode group with the build modes to use.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	int32 ResolveBuildModeInheritance(const int32 BuildModeGroupId) const;

	/**
	 * Find the root build mode group of the given build mode group, following inheritance.
	 *
	 * @param BuildModeGroup The build mode group to resolve.
	 */
	TSharedRef<FBGC_BuildModeGroup> ResolveBuildModeInheritance(
		const TSharedRef<FBGC_BuildModeGroup> BuildModeGroup
	) const;

	/**
	 * Retrieves the build mode group data for a specific hologram class.
	 *
	 * @param HologramClass The hologram class to search for.
	 */
	TSharedPtr<FBGC_BuildModeGroup> FindBuildModeGroupOf(const TSubclassOf<AFGHologram> HologramClass) const;

	/**
	 * Serializes the build mode group.
	 */
	FString BuildModeDataToJsonString();

	/**
	 * Get a JSON representation of the mutable parts of the build mode group.
	 */
	TSharedPtr<FJsonObject> BuildModeDataToJson();

	/**
	 * Saves the build mode data to disk.
	 */
	UFUNCTION(BlueprintCallable, Category = "BuildGunConfig|BuildModes")
	void SaveBuildModeData();

	/**
	 * Loads the build mode data from disk.
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
	void LoadBuildModeData(const TSharedRef<FJsonObject> JsonObject);

	/**
	 * Filter and sort the build modes for a hologram.
	 *
	 * @param BuildModeGroup The hologram data to use.
	 * @param BuildModes The build modes list, modified in place.
	 */
	void FilterAndSortBuildModes(
		TSharedRef<FBGC_BuildModeGroup> BuildModeGroup,
		TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& BuildModes
	) const;

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
};
