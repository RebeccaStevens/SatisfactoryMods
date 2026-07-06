#include "BGC_AbstractPlayerSubsystem.h"

#include <Policies/CondensedJsonPrintPolicy.h>
#include <Serialization/JsonSerializer.h>
#include <Serialization/JsonWriter.h>

#include "CoreMinimal.h"
#include "BGC_AbstractWorld.h"

#include "FGBuildGunModeDescriptor.h"
#include "FGGameUserSettings.h"
#include "FGRecipeManager.h"
#include "Hologram/FGHologram.h"
#include "Resources/FGBuildingDescriptor.h"
#include "Resources/FGItemDescriptor.h"
#include "Settings/FGUserSettingApplyType.h"

#include "BGC_Module.h"
#include "UserSettings/BGC_BuildModeGroup.h"

void ABGC_AbstractPlayerSubsystem::BeginPlay() {
	Super::BeginPlay();
	const auto WorldModule = UBGC_AbstractWorld::Get(this);
	if (!ensureMsgf(IsValid(WorldModule), TEXT("Failed to get the world module."))) {
		return;
	}
	WorldModule->PlayerSubsystem = this;
	RebuildBuildModeGroups();
}

TArray<int32> ABGC_AbstractPlayerSubsystem::GetBuildModeGroupIdsForDisplay() const {
	TArray<int32> Ids;
	Ids.Reserve(BuildModeGroups.Num());
	for (const auto& BuildModeGroup : BuildModeGroups) {
		if (!BuildModeGroup->IsEnabled) {
			continue;
		}
		if (BuildModeGroup->AppliesTo.IsEmpty()) {
			continue;
		}
		Ids.Add(BuildModeGroup->Id);
	}
	return Ids;
}

FBGC_BuildModeGroup& ABGC_AbstractPlayerSubsystem::GetBuildModeGroup(const int32 BuildModeGroupId) {
	checkf(BuildModeGroups.IsValidIndex(BuildModeGroupId), TEXT("Invalid Index"));
	return BuildModeGroups[BuildModeGroupId].Get();
}

TArray<TSubclassOf<UFGBuildGunModeDescriptor>> ABGC_AbstractPlayerSubsystem::GetSupportedBuildModes(
	const int32 BuildModeGroupId
) const {
	checkf(BuildModeGroups.IsValidIndex(BuildModeGroupId), TEXT("Invalid Index"));
	TArray<TSubclassOf<UFGBuildGunModeDescriptor>> Out;
	BuildModeGroups[BuildModeGroupId]->BuildModes.GetKeys(Out);
	return MoveTemp(Out);
}

void ABGC_AbstractPlayerSubsystem::EnsureValidBuildModeGroup(const int32 BuildModeGroupId) {
	checkf(BuildModeGroups.IsValidIndex(BuildModeGroupId), TEXT("Invalid Index"));
	EnsureValidBuildModeGroup(BuildModeGroups[BuildModeGroupId]);
}

void ABGC_AbstractPlayerSubsystem::EnsureValidBuildModeGroup(
	const TSharedRef<FBGC_BuildModeGroup> BuildModeGroup
) const {
	// Disable the build mode group if it has no build modes.
	if (BuildModeGroup->BuildModes.IsEmpty()) {
		BuildModeGroup->IsEnabled = false;
	}

	// If no build modes are enabled, enable them all.
	if (![&]() {
		for (const auto& [_, BuildMode] : BuildModeGroup->BuildModes) {
			if (BuildMode.IsEnabled) {
				return true;
			}
		}
		return false;
	}()) {
		for (auto& [_, Data] : BuildModeGroup->BuildModes) {
			Data.IsEnabled = true;
		}
	}

	// Make the display name and icon valid.
	if (BuildModeGroup->DisplayName.IsEmpty()) {
		if (BuildModeGroup->AppliesTo.IsValidIndex(0)) {
			BuildModeGroup->DisplayName = UFGItemDescriptor::GetItemName(BuildModeGroup->AppliesTo[0]);
		}
		if (BuildModeGroup->DisplayName.IsEmpty()) {
			BuildModeGroup->DisplayName = DefaultBuildModeDisplayName;
		}
	}
	if (!IsValid(BuildModeGroup->Icon)) {
		if (BuildModeGroup->AppliesTo.IsValidIndex(0)) {
			BuildModeGroup->Icon = UFGItemDescriptor::GetBigIcon(BuildModeGroup->AppliesTo[0]);
		}
		if (!IsValid(BuildModeGroup->Icon)) {
			BuildModeGroup->Icon = DefaultBuildModeIcon;
		}
	}
}

void ABGC_AbstractPlayerSubsystem::EnsureValidBuildModeGroupBuildModes(const int32 BuildModeGroupId) {
	checkf(BuildModeGroups.IsValidIndex(BuildModeGroupId), TEXT("Invalid Index"));
	EnsureValidBuildModeGroupBuildModes(BuildModeGroups[BuildModeGroupId]);
}

void ABGC_AbstractPlayerSubsystem::EnsureValidBuildModeGroupBuildModes(
	const TSharedRef<FBGC_BuildModeGroup> BuildModeGroup
) {
	// If no build modes are enabled, enable them all.
	if (![&]() {
		for (const auto& [_, BuildMode] : BuildModeGroup->BuildModes) {
			if (BuildMode.IsEnabled) {
				return true;
			}
		}
		return false;
	}()) {
		for (auto& [_, Data] : BuildModeGroup->BuildModes) {
			Data.IsEnabled = true;
		}
	}
}

void ABGC_AbstractPlayerSubsystem::ResetBuildModeData() {
	for (const auto& BuildModeGroup : BuildModeGroups) {
		RemoveBuildModeGroupData(BuildModeGroup);
	}
}

void ABGC_AbstractPlayerSubsystem::RemoveBuildModeGroupData(const int32 BuildModeGroupId) {
	checkf(BuildModeGroups.IsValidIndex(BuildModeGroupId), TEXT("Invalid Index"));
	RemoveBuildModeGroupData(BuildModeGroups[BuildModeGroupId]);
}

void ABGC_AbstractPlayerSubsystem::RemoveBuildModeGroupData(const TSharedRef<FBGC_BuildModeGroup> BuildModeGroup) {
	BuildModeGroup->Reset();
}

void ABGC_AbstractPlayerSubsystem::SetBuildModeGroupInherits(const int32 BuildModeGroupId, const bool ShouldInherit) {
	checkf(BuildModeGroups.IsValidIndex(BuildModeGroupId), TEXT("Invalid Index"));
	const auto BuildModeGroup = BuildModeGroups[BuildModeGroupId];
	BuildModeGroup->InheritBuildModes = ShouldInherit;
	checkf(
		!ShouldInherit || BuildModeGroup->InheritBuildModesFrom != INDEX_NONE,
		TEXT("Should not set ShouldInherit of something that can't inherit.")
	);
}

void ABGC_AbstractPlayerSubsystem::SetBuildModeGroupBuildMode(
	const int32 BuildModeGroupId,
	const TSubclassOf<UFGBuildGunModeDescriptor> BuildModeClass,
	const FBGC_BuildModeGroup_BuildMode& BuildModeData
) {
	checkf(BuildModeGroups.IsValidIndex(BuildModeGroupId), TEXT("Invalid Index"));
	BuildModeGroups[BuildModeGroupId]->BuildModes.Add(BuildModeClass, BuildModeData);
}

void ABGC_AbstractPlayerSubsystem::RebuildBuildModeGroups() {
	InitializeBuildModeGroups();
	const auto BuildingDescriptorsByHologram = GetBuildingDescriptorsByHologram();
	const auto HologramsByBuildModes = GetHologramsByBuildModes(BuildingDescriptorsByHologram);
	const auto BuildModeGroupsData = PopulateBuildModeGroups(HologramsByBuildModes);

	for (auto BuildModeGroupIndex = 0; BuildModeGroupIndex < BuildModeGroups.Num(); BuildModeGroupIndex++) {
		const auto BuildModeGroup = BuildModeGroups[BuildModeGroupIndex];
		BuildModeGroup->Id = BuildModeGroupIndex;

		const auto [Holograms, BuildModes] = BuildModeGroupsData.FindRef(BuildModeGroup.ToWeakPtr());
		for (const auto& HologramClass : Holograms) {
			if (const auto BuildingDescriptors = BuildingDescriptorsByHologram->Find(HologramClass)) {
				BuildModeGroup->AppliesTo.Append(*BuildingDescriptors);
			}
		}
		BuildModeGroup->AppliesTo.StableSort(
			[](const auto& A, const auto& B) {
				return UFGItemDescriptor::GetMenuPriority(A) < UFGItemDescriptor::GetMenuPriority(B);
			}
		);

		check(BuildModeGroup->BuildModes.IsEmpty());
		if (BuildModes.IsEmpty()) {
			BuildModeGroup->IsEnabled = false;
		} else {
			for (auto BuildModeIndex = 0; BuildModeIndex < BuildModes.Num(); BuildModeIndex++) {
				BuildModeGroup->BuildModes.Add(BuildModes[BuildModeIndex]).Weight = BuildModeIndex;
			}
		}
	}

	LoadBuildModeData();
}

void ABGC_AbstractPlayerSubsystem::InitializeBuildModeGroups() {
	BuildModeGroups.Empty();
	BuildModeGroupsByHologram.Empty();

	// First pass - Create the predefined build mode groups and add their direct hologram classes to the lookup table.
	for (const auto& PredefinedBuildModeGroup : PredefinedBuildModeGroups) {
		const auto& BuildModeGroup = BuildModeGroups.Add_GetRef(PredefinedBuildModeGroup.CreateBuildModeGroup());
		for (const auto& HologramClass : PredefinedBuildModeGroup.Holograms) {
			BuildModeGroupsByHologram.Add(HologramClass, BuildModeGroup.ToWeakPtr());
		}
	}

	// Second pass - Add all derived hologram classes to the lookup table, pointing to the same built mode group as the closest parent class.
	TArray<UClass*> DerivedHologramClasses;
	TArray<TSubclassOf<AFGHologram>> BaseHolograms;
	BuildModeGroupsByHologram.GenerateKeyArray(BaseHolograms);
	for (const auto& Hologram : BaseHolograms) {
		DerivedHologramClasses.Empty();
		GetDerivedClasses(Hologram, DerivedHologramClasses);
		for (const auto DerivedHologram : DerivedHologramClasses) {
			// Skip if already stored.
			if (BuildModeGroupsByHologram.Contains(DerivedHologram)) {
				continue;
			}

			// Find the closest parent class that is stored.
			TWeakPtr<FBGC_BuildModeGroup> BuildModeGroup;
			{
				UClass* ParentClass = DerivedHologram->GetSuperClass();
				while (ParentClass != nullptr) {
					BuildModeGroup = BuildModeGroupsByHologram.FindRef(ParentClass);
					if (BuildModeGroup.IsValid()) {
						break;
					}
					ParentClass = ParentClass->GetSuperClass();
				}
				if (!BuildModeGroup.IsValid()) {
					UE_LOG(
						LogBuildGunConfig,
						Warning,
						TEXT("Could not find stored parent for hologram %s. Skipping."),
						*DerivedHologram->GetName()
					);
					continue;
				}
			}

			BuildModeGroupsByHologram.Add(DerivedHologram, BuildModeGroup);
		}
	}
}

TSharedRef<TMap<TSubclassOf<AFGHologram>, TArray<TSubclassOf<UFGBuildingDescriptor>>>>
ABGC_AbstractPlayerSubsystem::GetBuildingDescriptorsByHologram() {
	auto BuildingDescriptorsByHologram = MakeShared<TMap<
		TSubclassOf<AFGHologram>, TArray<TSubclassOf<UFGBuildingDescriptor>>>>();

	const TObjectPtr<AFGRecipeManager> RecipeManager = AFGRecipeManager::Get(this);
	ensureMsgf(IsValid(RecipeManager), TEXT("Failed to get recipe manager."));
	const auto ShowLocked = ShouldShowLockedBuildables();

	TArray<UClass*> BuildingDescriptorUClasses;
	GetDerivedClasses(UFGBuildingDescriptor::StaticClass(), BuildingDescriptorUClasses, true);
	for (const auto BuildingDescriptorUClass : BuildingDescriptorUClasses) {
		const auto BuildingDescriptorClass = static_cast<TSubclassOf<UFGBuildingDescriptor>>(BuildingDescriptorUClass);
		const auto HologramClass = UFGBuildDescriptor::GetHologramClass(*BuildingDescriptorClass);
		// If the hologram class is null, or if we are not showing locked buildables and there are no unlocked recipes for the building descriptor, skip it.
		if (HologramClass == nullptr || (!ShowLocked && IsValid(RecipeManager) && RecipeManager->FindRecipesByProduct(
			BuildingDescriptorClass,
			true
		).Num() == 0)) {
			continue;
		}

		BuildingDescriptorsByHologram->FindOrAdd(HologramClass).Add(*BuildingDescriptorClass);
	}
	return MoveTemp(BuildingDescriptorsByHologram);
}

TSharedRef<TMap<TArray<TSubclassOf<UFGBuildGunModeDescriptor>>, TSet<TSubclassOf<AFGHologram>>>>
ABGC_AbstractPlayerSubsystem::GetHologramsByBuildModes(
	const TSharedRef<TMap<TSubclassOf<AFGHologram>, TArray<TSubclassOf<UFGBuildingDescriptor>>>>
	BuildingDescriptorsByHologram
) {
	auto HologramsByBuildModes = MakeShared<TMap<
		TArray<TSubclassOf<UFGBuildGunModeDescriptor>>, TSet<TSubclassOf<AFGHologram>>>>();

	for (const auto& [HologramClass, BuildingDescriptorClasses] : *BuildingDescriptorsByHologram) {
		const auto& HologramDefaultObject = *HologramClass.GetDefaultObject();

		TArray<TSubclassOf<UFGBuildGunModeDescriptor>> SupportedBuildModes;
		HologramDefaultObject.GetSupportedBuildModes_Implementation(SupportedBuildModes);
		if (SupportedBuildModes.Num() > 1) {
			HologramsByBuildModes->FindOrAdd(SupportedBuildModes).Add(HologramClass);
		}
	}

	return MoveTemp(HologramsByBuildModes);
}

TMap<TWeakPtr<FBGC_BuildModeGroup>, ABGC_AbstractPlayerSubsystem::FBGC_BuildModeGroupData>
ABGC_AbstractPlayerSubsystem::PopulateBuildModeGroups(
	const TSharedRef<TMap<TArray<TSubclassOf<UFGBuildGunModeDescriptor>>, TSet<TSubclassOf<AFGHologram>>>> HologramsByBuildModes
) {
	TMap<TWeakPtr<FBGC_BuildModeGroup>, FBGC_BuildModeGroupData> ResultByBuildModeGroup;
	for (const auto& [BuildModes, Holograms] : HologramsByBuildModes.Get()) {
		TWeakPtr<FBGC_BuildModeGroup> NewBuildModeGroup;
		for (const auto& Hologram : Holograms) {
			if (const auto FoundBuildModeGroup = BuildModeGroupsByHologram.FindRef(*Hologram);
				FoundBuildModeGroup.IsValid()) {
				auto& Result = ResultByBuildModeGroup.FindOrAdd(FoundBuildModeGroup);
				Result.Holograms.Add(Hologram);
				Result.BuildModes = BuildModes;
				continue;
			}

			if (!NewBuildModeGroup.IsValid()) {
				auto TempBuildModeGroup = MakeShared<FBGC_BuildModeGroup>();
				TempBuildModeGroup->Id = BuildModeGroups.Num();
				NewBuildModeGroup = BuildModeGroups.Add_GetRef(MoveTemp(TempBuildModeGroup)).ToWeakPtr();
			}

			BuildModeGroupsByHologram.Add(*Hologram, NewBuildModeGroup);
			auto& Result = ResultByBuildModeGroup.FindOrAdd(NewBuildModeGroup);
			Result.Holograms.Add(Hologram);
			Result.BuildModes = BuildModes;
		}
	}

	return MoveTemp(ResultByBuildModeGroup);
}

int32 ABGC_AbstractPlayerSubsystem::GetBuildModeGroupInheritFrom(const int32 BuildModeGroupId) const {
	checkf(BuildModeGroups.IsValidIndex(BuildModeGroupId), TEXT("Invalid Index"));
	const auto InheritFrom = GetBuildModeGroupInheritFrom(BuildModeGroups[BuildModeGroupId]);
	return InheritFrom.IsValid() ? InheritFrom->Id : INDEX_NONE;
}

TSharedPtr<FBGC_BuildModeGroup> ABGC_AbstractPlayerSubsystem::GetBuildModeGroupInheritFrom(
	const TSharedRef<FBGC_BuildModeGroup> BuildModeGroup
) const {
	TSharedRef<FBGC_BuildModeGroup> Resolved = BuildModeGroup;
	TSet<int32> Visited;
	while (true) {
		if (!ensureMsgf(!Visited.Contains(Resolved->Id), TEXT("Circular inheritance."))) {
			return nullptr;
		}
		Visited.Add(Resolved->Id);
		if (Resolved->InheritBuildModesFrom == INDEX_NONE) {
			return nullptr;
		}
		if (!ensureMsgf(
			Resolved->InheritBuildModesFrom >= 0 && Resolved->InheritBuildModesFrom < BuildModeGroups.Num(),
			TEXT("Index out of range (%i)"),
			Resolved->InheritBuildModesFrom
		)) {
			return nullptr;
		}
		Resolved = BuildModeGroups[Resolved->InheritBuildModesFrom];
		if (Resolved->IsEnabled) {
			return Resolved.ToSharedPtr();
		}
	}
}

int32 ABGC_AbstractPlayerSubsystem::ResolveBuildModeInheritance(const int32 BuildModeGroupId) const {
	checkf(BuildModeGroups.IsValidIndex(BuildModeGroupId), TEXT("Invalid Index"));
	return ResolveBuildModeInheritance(BuildModeGroups[BuildModeGroupId])->Id;
}

TSharedRef<FBGC_BuildModeGroup> ABGC_AbstractPlayerSubsystem::ResolveBuildModeInheritance(
	const TSharedRef<FBGC_BuildModeGroup> BuildModeGroup
) const {
	TSharedRef<FBGC_BuildModeGroup> Resolved = BuildModeGroup;
	TSet<int32> Visited;
	while (true) {
		if (!ensureMsgf(!Visited.Contains(Resolved->Id), TEXT("Circular inheritance."))) {
			return BuildModeGroup;
		}
		Visited.Add(Resolved->Id);
		if (Resolved->InheritBuildModesFrom == INDEX_NONE || !Resolved->InheritBuildModes) {
			return Resolved;
		}
		if (!ensureMsgf(
			Resolved->InheritBuildModesFrom >= 0 && Resolved->InheritBuildModesFrom < BuildModeGroups.Num(),
			TEXT("Index out of range (%i)"),
			Resolved->InheritBuildModesFrom
		)) {
			return Resolved;
		}
		const auto InheritFrom = BuildModeGroups[Resolved->InheritBuildModesFrom];
		if (!InheritFrom->IsEnabled) {
			return Resolved;
		}
		Resolved = InheritFrom;
	}
}

TSharedPtr<FBGC_BuildModeGroup> ABGC_AbstractPlayerSubsystem::FindBuildModeGroupOf(
	const TSubclassOf<AFGHologram> HologramClass
) const {
	const auto BuildModeGroupWeakPtr = BuildModeGroupsByHologram.FindRef(HologramClass);
	if (!BuildModeGroupWeakPtr.IsValid()) {
		return nullptr;
	}
	const auto BuildModeGroupPtr = BuildModeGroupWeakPtr.Pin();
	if (!BuildModeGroupPtr.IsValid()) {
		return nullptr;
	}
	return ResolveBuildModeInheritance(BuildModeGroupPtr.ToSharedRef());
}

FString ABGC_AbstractPlayerSubsystem::BuildModeDataToJsonString() {
	const TSharedPtr<FJsonObject> RootObject = BuildModeDataToJson();
	if (!RootObject.IsValid()) {
		UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to serialize build mode data."));
		return FString();
	}

	FString JsonString;
	const auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	return JsonString;
}

TSharedPtr<FJsonObject> ABGC_AbstractPlayerSubsystem::BuildModeDataToJson() {
	TSharedPtr<FJsonObject> BuildModeDataJsonObject = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> BuildModeGroupsArray;
	for (const auto& BuildModeGroup : BuildModeGroups) {
		BuildModeGroupsArray.Add(MakeShareable(new FJsonValueObject(BuildModeGroup->ToJson())));
	}
	BuildModeDataJsonObject->SetArrayField(TEXT("BuildModeGroups"), BuildModeGroupsArray);

	return BuildModeDataJsonObject;
}

void ABGC_AbstractPlayerSubsystem::SaveBuildModeData() {
	const UFGGameUserSettings* UserSettings = UFGGameUserSettings::GetFGGameUserSettings();
	if (!IsValid(UserSettings)) {
		UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to get user settings."));
		return;
	}
	const auto BuildModesSetting = UserSettings->FindUserSetting("BuildGunConfig.Json");
	if (!IsValid(BuildModesSetting)) {
		UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to find build modes setting."));
		return;
	}
	const FString JsonString = BuildModeDataToJsonString();
	if (JsonString.IsEmpty()) {
		UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to serialize build mode data."));
		return;
	}
	BuildModesSetting->ForceSetValue(JsonString);
	BuildModesSetting->MarkDirty();
}

void ABGC_AbstractPlayerSubsystem::LoadBuildModeData() {
	const UFGGameUserSettings* UserSettings = UFGGameUserSettings::GetFGGameUserSettings();
	if (!IsValid(UserSettings)) {
		UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to get user settings."));
		return;
	}
	const auto BuildModes = UserSettings->GetStringOptionValue("BuildGunConfig.Json");
	LoadBuildModeData(BuildModes);
}

void ABGC_AbstractPlayerSubsystem::LoadBuildModeData(const FString& JsonString) {
	if (JsonString.IsEmpty()) {
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	if (const auto Reader = TJsonReaderFactory<>::Create(JsonString);
		!FJsonSerializer::Deserialize(Reader, JsonObject)) {
		UE_LOG(LogBuildGunConfig, Error, TEXT("Failed to deserialize JSON string."));
		return;
	}

	if (!JsonObject.IsValid()) {
		UE_LOG(LogBuildGunConfig, Error, TEXT("JSON object is invalid."));
		return;
	}

	LoadBuildModeData(JsonObject.ToSharedRef());
}

void ABGC_AbstractPlayerSubsystem::LoadBuildModeData(const TSharedRef<FJsonObject> JsonObject) {
	const TArray<TSharedPtr<FJsonValue>>* BuildModeGroupsArray;
	if (!JsonObject->TryGetArrayField(TEXT("BuildModeGroups"), BuildModeGroupsArray)) {
		UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to load build mode data: BuildModeGroups array is invalid."));
		return;
	}

	for (const auto& BuildModeGroupEntry : *BuildModeGroupsArray) {
		if (!BuildModeGroupEntry.IsValid()) {
			UE_LOG(LogBuildGunConfig, Warning, TEXT("Invalid build mode group entry... skipping."));
			continue;
		}
		const auto BuildModeGroupObject = BuildModeGroupEntry->AsObject();
		if (!BuildModeGroupObject.IsValid()) {
			UE_LOG(LogBuildGunConfig, Warning, TEXT("Invalid build mode group object... skipping."));
			continue;
		}

		FBGC_BuildModeGroup::LoadFromJson(BuildModeGroupObject.ToSharedRef(), BuildModeGroups);
	}
}

void ABGC_AbstractPlayerSubsystem::FilterAndSortBuildModes(
	const TSharedRef<FBGC_BuildModeGroup> BuildModeGroup,
	TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& BuildModes
) const {
	if (BuildModes.Num() == 0) {
		return;
	}

	auto FirstBuildMode = BuildModes[0];

	// Remove disabled build modes.
	BuildModes.RemoveAll(
		[&](const auto& BuildMode) {
			const auto Data = BuildModeGroup->BuildModes.Find(BuildMode);
			// Keep build modes that we don't have data entries for.
			if (Data == nullptr) {
				return false;
			}
			return !Data->IsEnabled;
		}
	);

	// Make sure we have at least one build mode.
	if (BuildModes.Num() == 0) {
		BuildModes.Add(MoveTemp(FirstBuildMode));
		return;
	}

	// Sort the build modes.
	BuildModes.StableSort(
		[&](const auto& A, const auto& B) {
			auto AData = BuildModeGroup->BuildModes.Find(A);
			auto BData = BuildModeGroup->BuildModes.Find(B);

			auto AIndex = AData == nullptr ? MAX_int32 : AData->Weight;
			auto BIndex = BData == nullptr ? MAX_int32 : BData->Weight;

			return AIndex < BIndex;
		}
	);
}

#if WITH_EDITOR
void ABGC_AbstractPlayerSubsystem::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) {
	Super::PostEditChangeChainProperty(Event);

	auto ActiveNode = Event.PropertyChain.GetTail();
	if (!ActiveNode) {
		return;
	}

	if (ActiveNode->GetValue()->GetFName() == GET_MEMBER_NAME_CHECKED(
		ABGC_AbstractPlayerSubsystem,
		PredefinedBuildModeGroups
	)) {
		if (Event.ChangeType == EPropertyChangeType::ValueSet) {
			const auto EditedIndex = Event.GetArrayIndex(ActiveNode->GetValue()->GetName());
			ActiveNode = ActiveNode->GetNextNode();
			if (!ActiveNode || EditedIndex == INDEX_NONE || ActiveNode->GetValue()->GetFName() !=
				GET_MEMBER_NAME_CHECKED(FBGC_PredefinedBuildModeGroup, InheritBuildModesFrom)) {
				return;
			}

			auto& Value = PredefinedBuildModeGroups[EditedIndex].InheritBuildModesFrom;
			if (Value != INDEX_NONE && (!PredefinedBuildModeGroups.IsValidIndex(Value) || Value == EditedIndex)) {
				UE_LOG(
					LogBuildGunConfig,
					Warning,
					TEXT(
						"Invalid InheritBuildModesFrom value. Must be a valid index different from the current one or INDEX_NONE."
					)
				);
				Value = INDEX_NONE;
			}

			return;
		}

		if (Event.ChangeType == EPropertyChangeType::ArrayMove || Event.ChangeType ==
			EPropertyChangeType::ArrayRemove) {
			// TODO: Update `InheritBuildModesFrom` values when moving elements in the array.
			// Get a mapping of old indexes to new indexes of each `PredefinedBuildModeGroups`.
			// Then, iterate over all `PredefinedBuildModeGroups` and update each `InheritBuildModesFrom` from the old index to the new index.
		}
	}
}
#endif
