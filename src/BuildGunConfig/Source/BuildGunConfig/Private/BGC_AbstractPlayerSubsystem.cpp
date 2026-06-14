#include "BGC_AbstractPlayerSubsystem.h"

#include "CoreMinimal.h"

#include "FGBuildGunModeDescriptor.h"
#include "FGGameUserSettings.h"
#include "FGRecipeManager.h"
#include "Hologram/FGHologram.h"
#include "Resources/FGBuildingDescriptor.h"
#include "Resources/FGItemDescriptor.h"
#include "Settings/FGUserSettingApplyType.h"

#include "Configuration/Properties/ConfigPropertySection.h"
#include "Kismet/RuntimeBlueprintFunctionLibrary.h"
#include "Module/WorldModuleManager.h"

#include "BGC_Module.h"
#include "UserSettings/BGC_BuildMode_Data.h"

void ABGC_AbstractPlayerSubsystem::BeginPlay() {
  Super::BeginPlay();
  auto WorldModule = UBGC_AbstractWorld::Get(this);
  if (!ensureMsgf(IsValid(WorldModule), TEXT("Failed to get the world module."))) {
    return;
  }
  WorldModule->PlayerSubsystem = this;
  LoadBuildModeData();
}

UTexture2D* ABGC_AbstractPlayerSubsystem::GetBuildModeIconChecked(TSubclassOf<AFGHologram> HologramClass) {
  auto& HologramData = BuildModesData.FindChecked(HologramClass);
  return HologramData.Icon;
}

FBGC_BuildMode_Data& ABGC_AbstractPlayerSubsystem::GetBuildModeDataChecked(TSubclassOf<AFGHologram> HologramClass) {
  return BuildModesData.FindChecked(HologramClass);
}

FBGC_BuildMode_DataEntry& ABGC_AbstractPlayerSubsystem::GetBuildModeDataEntryChecked(
  TSubclassOf<AFGHologram> HologramClass, TSubclassOf<UFGBuildGunModeDescriptor> BuildModeClass) {
  auto& HologramData = BuildModesData.FindChecked(HologramClass);
  return HologramData.BuildModes.FindChecked(BuildModeClass);
}

void ABGC_AbstractPlayerSubsystem::ResetBuildModeData(bool bSave) {
  for (auto& HologramData : BuildModesData) {
    HologramData.Value.Reset();
  }
  if (bSave) {
    SaveBuildModeData();
  }
}

void ABGC_AbstractPlayerSubsystem::RemoveBuildModeData(TSubclassOf<AFGHologram> HologramClass, bool bSave) {
  auto HologramData = BuildModesData.Find(HologramClass);
  if (HologramData == nullptr) {
    return;
  }
  HologramData->Reset();
  if (bSave) {
    SaveBuildModeData();
  }
}

void ABGC_AbstractPlayerSubsystem::RemoveBuildModeDataEntry(
  TSubclassOf<AFGHologram> HologramClass, TSubclassOf<UFGBuildGunModeDescriptor> BuildModeClass, bool bSave) {
  auto HologramData = BuildModesData.Find(HologramClass);
  if (HologramData == nullptr) {
    return;
  }
  HologramData->BuildModes.Remove(BuildModeClass);
  if (bSave) {
    SaveBuildModeData();
  }
}

void ABGC_AbstractPlayerSubsystem::SetBuildModeDataChecked(
  TSubclassOf<AFGHologram> HologramClass, const FBGC_BuildMode_Data& HologramBuildModeData, bool bSave) {
  auto& HologramData = BuildModesData.FindChecked(HologramClass);
  HologramData = HologramBuildModeData;
  if (bSave) {
    SaveBuildModeData();
  }
}

void ABGC_AbstractPlayerSubsystem::SetBuildModeDataEntryChecked(
  TSubclassOf<AFGHologram> HologramClass,
  TSubclassOf<UFGBuildGunModeDescriptor> BuildModeClass,
  const FBGC_BuildMode_DataEntry& HologramBuildModeDataEntry,
  bool bSave) {
  auto& HologramData = BuildModesData.FindChecked(HologramClass);
  HologramData.BuildModes.Add(BuildModeClass, HologramBuildModeDataEntry);
  if (bSave) {
    SaveBuildModeData();
  }
}

void ABGC_AbstractPlayerSubsystem::RebuildBuildModesData() {
  if (!IsValid(RecipeManager)) {
    RecipeManager = AFGRecipeManager::Get(this);
    if (!IsValid(RecipeManager)) {
      UE_LOG(LogBuildGunConfig, Error, TEXT("Failed to get recipe manager."));
      return;
    }
  }

  TMap<TSubclassOf<AFGHologram>, TArray<TSubclassOf<UFGBuildingDescriptor>>> HologramToBuildingDescriptorsMap;
  TMap<TSubclassOf<AFGHologram>, TArray<TSubclassOf<UFGBuildingDescriptor>>> AliasedHologramToBuildingDescriptorsMap;

  TArray<UClass*> BuildingDescriptorsUClass;
  GetDerivedClasses(UFGBuildingDescriptor::StaticClass(), BuildingDescriptorsUClass, true);

  auto ShowLocked = ShowLockedBuildables();

  for (auto BuildingDescriptorUClass : BuildingDescriptorsUClass) {
    auto BuildingDescriptor = (TSubclassOf<UFGBuildingDescriptor>)BuildingDescriptorUClass;
    auto HologramClass = UFGBuildDescriptor::GetHologramClass(*BuildingDescriptor);
    if (HologramClass == nullptr) {
      continue;
    }
    if (!ShowLocked && RecipeManager->FindRecipesByProduct(BuildingDescriptor, true).Num() == 0) {
      continue;
    }

    HologramToBuildingDescriptorsMap.FindOrAdd(HologramClass).AddUnique(*BuildingDescriptor);
  }

  for (auto& Pair : HologramToBuildingDescriptorsMap) {
    auto& HologramClass = Pair.Key;
    auto Hologram = HologramClass.GetDefaultObject();
    if (Hologram == nullptr) {
      continue;
    }

    TArray<TSubclassOf<UFGBuildGunModeDescriptor>> SupportedBuildModes;
    Hologram->GetSupportedBuildModes_Implementation(SupportedBuildModes);
    if (SupportedBuildModes.Num() <= 1) {
      continue;
    }

    auto& BuildingDescriptors = Pair.Value;
    if (BuildingDescriptors.Num() == 0) {
      continue;
    }
    BuildingDescriptors.StableSort(
      [](auto& A, auto& B) { return UFGItemDescriptor::GetMenuPriority(A) < UFGItemDescriptor::GetMenuPriority(B); });

    // Add valid aliases to the aliases map for handling later.
    if (BuildModesAliases.Contains(HologramClass)) {
      auto& AliasClass = BuildModesAliases.FindChecked(HologramClass);
      auto AliasHologram = AliasClass.GetDefaultObject();
      if (AliasHologram == nullptr) {
        continue;
      }

      TArray<TSubclassOf<UFGBuildGunModeDescriptor>> SupportedAliasBuildModes;
      AliasHologram->GetSupportedBuildModes_Implementation(SupportedAliasBuildModes);

      if (SupportedAliasBuildModes != SupportedBuildModes) {
        UE_LOG(
          LogBuildGunConfig,
          Warning,
          TEXT("Alias %s has different supported build modes than its target %s. Skipping alias."),
          *AliasClass->GetName(),
          *HologramClass->GetName());
        continue;
      }

      auto& AliasedBuildingDescriptors = AliasedHologramToBuildingDescriptorsMap.FindOrAdd(HologramClass);
      for (auto BuildingDescriptor : BuildingDescriptors) {
        AliasedBuildingDescriptors.AddUnique(BuildingDescriptor);
      }
      continue;
    }

    auto& BuildModeData = BuildModesData.FindOrAdd(HologramClass);
    if (!BuildModeData.IsEnabled) {
      continue;
    }
    if (BuildModeData.DisplayName.IsEmpty()) {
      UE_LOG(LogBuildGunConfig, Verbose, TEXT("Initializing %s display name."), *HologramClass->GetName());
      BuildModeData.DisplayName = UFGItemDescriptor::GetItemName(*BuildingDescriptors[0]);
    }
    if (!IsValid(BuildModeData.Icon)) {
      BuildModeData.Icon = UFGItemDescriptor::GetBigIcon(*BuildingDescriptors[0]);
    }

    BuildModeData.AppliesTo = BuildingDescriptors;

    for (auto Index = 0; Index < SupportedBuildModes.Num(); Index++) {
      auto BuildModeClass = SupportedBuildModes[Index];
      auto BuildModeDataEntry = BuildModeData.BuildModes.Find(BuildModeClass);
      if (BuildModeDataEntry == nullptr) {
        FBGC_BuildMode_DataEntry NewEntry;
        NewEntry.Index = Index;
        NewEntry.IsEnabled = true;
        BuildModeData.BuildModes.Add(BuildModeClass, NewEntry);
      }
    }
  }

  for (auto& BuildingDescriptor : AliasedHologramToBuildingDescriptorsMap) {
    auto& HologramClass = BuildingDescriptor.Key;
    auto& BuildingDescriptors = BuildingDescriptor.Value;
    auto& AliasClass = BuildModesAliases.FindChecked(HologramClass);
    auto& BuildModeData = BuildModesData.FindChecked(AliasClass);
    BuildModeData.AppliesTo.Append(BuildingDescriptors);
  }
}

const FBGC_BuildMode_Data& ABGC_AbstractPlayerSubsystem::ResolveBuildModeInheritance(
  TSubclassOf<AFGHologram> HologramClass, const FBGC_BuildMode_Data& HologramBuildModeData) {
  if (!IsValid(HologramBuildModeData.InheritBuildModesFrom)) {
    return HologramBuildModeData;
  }

  auto InheritedData = BuildModesData.Find(HologramBuildModeData.InheritBuildModesFrom);
  if (InheritedData == nullptr) {
    return HologramBuildModeData;
  }

  return *InheritedData;
}

bool ABGC_AbstractPlayerSubsystem::ResolveBuildModeData(
  TSubclassOf<AFGHologram> HologramClass, FBGC_BuildMode_Data& Out_BuildModeData) {
  auto HologramBuildModeData = BuildModesData.Find(HologramClass);
  if (HologramBuildModeData == nullptr) {
    auto AliasClass = BuildModesAliases.Find(HologramClass);
    if (AliasClass == nullptr) {
      return false;
    }
    HologramBuildModeData = BuildModesData.Find(*AliasClass);
    if (HologramBuildModeData == nullptr) {
      return false;
    }
  }
  Out_BuildModeData = ResolveBuildModeInheritance(HologramClass, *HologramBuildModeData);
  return true;
}

void ABGC_AbstractPlayerSubsystem::SaveBuildModeData() {
  auto World = GetWorld();
  if (!ensureMsgf(IsValid(World), TEXT("Failed to get the world."))) {
    return;
  }

  UFGGameUserSettings* UserSettings = UFGGameUserSettings::GetFGGameUserSettings();
  auto BuildModesSetting = UserSettings->FindUserSetting("BuildGunConfig.BuildModes");
  if (!IsValid(BuildModesSetting)) {
    UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to find build modes setting."));
    return;
  }
  FVariant Data = BuildModesDataToJsonString();
  BuildModesSetting->ForceSetValue(Data);
  BuildModesSetting->MarkDirty();
}

FString ABGC_AbstractPlayerSubsystem::BuildModesDataToJsonString() {
  TSharedPtr<FJsonObject> RootObject = BuildModesDataToJson();
  if (!RootObject.IsValid()) {
    UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to serialize build mode data."));
    return FString();
  }

  FString JsonString;
  auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonString);
  FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

  return JsonString;
}

TSharedPtr<FJsonObject> ABGC_AbstractPlayerSubsystem::BuildModesDataToJson() {
  TSharedPtr<FJsonObject> HologramsObject = MakeShareable(new FJsonObject());
  for (const auto& HologramEntry : BuildModesData) {
    if (!ensureMsgf(HologramEntry.Key != nullptr, TEXT("Skipping null hologram key during serialization."))) {
      continue;
    }

    HologramsObject->SetObjectField(HologramEntry.Key->GetPathName(), HologramEntry.Value.ToJson());
  }

  return HologramsObject;
}

void ABGC_AbstractPlayerSubsystem::LoadBuildModeData() {
  auto World = GetWorld();
  if (!ensureMsgf(IsValid(World), TEXT("Failed to get the world."))) {
    return;
  }

  UFGGameUserSettings* UserSettings = UFGGameUserSettings::GetFGGameUserSettings();
  auto BuildModes = UserSettings->GetStringOptionValue("BuildGunConfig.BuildModes");
  LoadBuildModeData(BuildModes);
}

void ABGC_AbstractPlayerSubsystem::LoadBuildModeData(const FString& JsonString) {
  if (JsonString.IsEmpty()) {
    return;
  }

  TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
  auto Reader = TJsonReaderFactory<>::Create(JsonString);

  if (!FJsonSerializer::Deserialize(Reader, JsonObject)) {
    UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to deserialize JSON string."));
    return;
  }

  LoadBuildModeData(JsonObject);
}

void ABGC_AbstractPlayerSubsystem::LoadBuildModeData(TSharedPtr<FJsonObject> JsonObject) {
  if (!JsonObject.IsValid()) {
    UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to load build mode data: JsonObject is invalid."));
    return;
  }

  for (const auto& HologramEntry : JsonObject->Values) {
    auto HologramClass = LoadClass<AFGHologram>(nullptr, *HologramEntry.Key);
    if (HologramClass == nullptr) {
      UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to load hologram class with path %s"), *HologramEntry.Key);
      continue;
    }

    auto HologramDataObject = HologramEntry.Value->AsObject();
    if (HologramDataObject == nullptr) {
      continue;
    }

    FBGC_BuildMode_Data& HologramData = BuildModesData.FindOrAdd(HologramClass);
    auto LoadedHologramData = FBGC_BuildMode_Data::FromJson(HologramDataObject);
    HologramData.InheritBuildModes = LoadedHologramData.InheritBuildModes;
    HologramData.BuildModes = LoadedHologramData.BuildModes;
  }
}

void ABGC_AbstractPlayerSubsystem::FilterAndSortBuildModes(
  const FBGC_BuildMode_Data& HologramBuildModeData,
  TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& Out_BuildModes) const {
  if (Out_BuildModes.Num() == 0) {
    return;
  }

  auto FirstBuildMode = Out_BuildModes[0];

  // Remove disabled build modes.
  Out_BuildModes.RemoveAll([&](auto& BuildMode) {
    auto Data = HologramBuildModeData.BuildModes.Find(BuildMode);
    // Keep build modes that we don't have data entries for.
    if (Data == nullptr) {
      return false;
    }
    return !Data->IsEnabled;
  });

  // Make sure we have at least one build mode.
  if (Out_BuildModes.Num() == 0) {
    Out_BuildModes.Add(FirstBuildMode);
    return;
  }

  // Sort the build modes.
  Out_BuildModes.StableSort([&](auto& A, auto& B) {
    auto AData = HologramBuildModeData.BuildModes.Find(A);
    auto BData = HologramBuildModeData.BuildModes.Find(B);

    auto AIndex = AData == nullptr ? MAX_int32 : AData->Index;
    auto BIndex = BData == nullptr ? MAX_int32 : BData->Index;

    return AIndex < BIndex;
  });
}

#if WITH_EDITOR

EDataValidationResult
ABGC_AbstractPlayerSubsystem::ValidateBuildModeInheritance(FDataValidationContext& Context) const {
  EDataValidationResult ValidationResult = EDataValidationResult::Valid;

  for (auto& HologramBuildModeData : BuildModesData) {
    if (!IsValid(HologramBuildModeData.Value.InheritBuildModesFrom)) {
      continue;
    }

    // Validate that the build mode inherits from a valid hologram class.
    auto InheritedData = BuildModesData.Find(HologramBuildModeData.Value.InheritBuildModesFrom);

    // If the inherited data is not found, it's an error.
    if (InheritedData == nullptr) {
      ValidationResult = EDataValidationResult::Invalid;
      Context.AddError(
        FText::FromString(
          FString::Printf(
            TEXT("Hologram class %s inherits build modes from %s but it was not found in the build mode data."),
            *HologramBuildModeData.Key->GetName(),
            *HologramBuildModeData.Value.InheritBuildModesFrom->GetName())));
      continue;
    }

    if (InheritedData->InheritBuildModesFrom != nullptr) {
      // If the inherited data also inherits from another hologram, it's an error.
      ValidationResult = EDataValidationResult::Invalid;
      Context.AddError(
        FText::FromString(
          FString::Printf(
            TEXT(
              "Hologram class %s inherits build modes from %s which also inherits from another hologram."
              "Only single level inheritance is supported."),
            *HologramBuildModeData.Key->GetName(),
            *HologramBuildModeData.Value.InheritBuildModesFrom->GetName())));
      continue;
    }
  }

  return ValidationResult;
}

EDataValidationResult ABGC_AbstractPlayerSubsystem::IsDataValid(FDataValidationContext& Context) const {
  EDataValidationResult ValidationResult = Super::IsDataValid(Context);

  // If nothing has performed any validation yet, start with a valid result.
  if (ValidationResult == EDataValidationResult::NotValidated) {
    ValidationResult = EDataValidationResult::Valid;
  }

  if (BuildModesData.Find(nullptr) != nullptr) {
    Context.AddError(FText::FromString(TEXT("Build modes data contains nullptr key")));
    ValidationResult = EDataValidationResult::Invalid;
  }

  ValidationResult = ValidateBuildModeInheritance(Context);

  for (const auto& HologramEntry : BuildModesAliases) {
    if (HologramEntry.Key == nullptr || HologramEntry.Value == nullptr) {
      Context.AddError(FText::FromString(TEXT("Hologram alias contains nullptr key or value")));
      ValidationResult = EDataValidationResult::Invalid;
      continue;
    }
    if (BuildModesData.Find(HologramEntry.Value) == nullptr) {
      Context.AddError(
        FText::FromString(
          FString::Printf(
            TEXT("Hologram class %s is aliased to %s but it was not found in the build mode data."),
            *HologramEntry.Key->GetName(),
            *HologramEntry.Value->GetName())));
      ValidationResult = EDataValidationResult::Invalid;
    }
  }

  for (const auto& HologramEntry : BuildModesData) {
    if (BuildModesAliases.Find(HologramEntry.Key) != nullptr) {
      Context.AddError(
        FText::FromString(
          FString::Printf(
            TEXT("Hologram class %s both has build mode data and is aliased to another hologram."),
            *HologramEntry.Key->GetName())));
      ValidationResult = EDataValidationResult::Invalid;
    }
  }

  return ValidationResult;
}

void ABGC_AbstractPlayerSubsystem::SortAndReindexBuildModesData() {
  BuildModesData.ValueStableSort(
    [](const FBGC_BuildMode_Data& A, const FBGC_BuildMode_Data& B) { return A.EditorWeight < B.EditorWeight; });

  auto nextWeight = 0.5;
  for (auto& Entry : BuildModesData) {
    Entry.Value.EditorWeight = nextWeight++;
  }
}

void ABGC_AbstractPlayerSubsystem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
  Super::PostEditChangeProperty(PropertyChangedEvent);

  FName PropertyName = PropertyChangedEvent.GetPropertyName();
  FName MemberName = PropertyChangedEvent.GetMemberPropertyName();

  if (MemberName == GET_MEMBER_NAME_CHECKED(ABGC_AbstractPlayerSubsystem, BuildModesData)) {
    if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd) {
      TArray<TSubclassOf<AFGHologram>> Keys;
      BuildModesData.GetKeys(Keys);
      auto& NewEntry = BuildModesData.FindChecked(Keys.Last());
      NewEntry.EditorWeight = BuildModesData.Num() - 0.5;
    }

    if (
      PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayAdd ||
      PropertyChangedEvent.ChangeType == EPropertyChangeType::ArrayRemove ||
      (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet &&
       PropertyName == GET_MEMBER_NAME_CHECKED(FBGC_BuildMode_Data, EditorWeight))) {
      SortAndReindexBuildModesData();
    }
  }
}
#endif
