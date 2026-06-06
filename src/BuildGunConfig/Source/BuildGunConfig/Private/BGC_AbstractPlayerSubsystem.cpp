#include "BGC_AbstractPlayerSubsystem.h"

#include "CoreMinimal.h"

#include "Configuration/Properties/ConfigPropertyRaw.h"
#include "Configuration/Properties/ConfigPropertySection.h"
#include "Kismet/RuntimeBlueprintFunctionLibrary.h"
#include "Module/WorldModuleManager.h"

#include "FGBuildGunModeDescriptor.h"
#include "FGRecipeManager.h"
#include "Hologram/FGHologram.h"
#include "Resources/FGBuildingDescriptor.h"
#include "Resources/FGItemDescriptor.h"

#include "BGC_Module.h"
#include "Config/BGC_BuildMode_Data.h"

void ABGC_AbstractPlayerSubsystem::BeginPlay() {
  Super::BeginPlay();
  LoadBuildModeDataFromConfig();
}

UTexture2D* ABGC_AbstractPlayerSubsystem::GetBuildModeIconChecked(TSubclassOf<AFGHologram> HologramClass) {
  auto& hologramData = BuildModesData.FindChecked(HologramClass);
  return hologramData.Icon;
}

FBGC_BuildMode_Data& ABGC_AbstractPlayerSubsystem::GetBuildModeDataChecked(TSubclassOf<AFGHologram> HologramClass) {
  return BuildModesData.FindChecked(HologramClass);
}

FBGC_BuildMode_DataEntry& ABGC_AbstractPlayerSubsystem::GetBuildModeDataEntryChecked(
  TSubclassOf<AFGHologram> HologramClass, TSubclassOf<UFGBuildGunModeDescriptor> BuildModeClass) {
  auto& hologramData = BuildModesData.FindChecked(HologramClass);
  return hologramData.BuildModes.FindChecked(BuildModeClass);
}

void ABGC_AbstractPlayerSubsystem::RemoveBuildModeData(TSubclassOf<AFGHologram> HologramClass) {
  // Don't remove the entry, just clear the relevant data.
  auto hologramData = BuildModesData.Find(HologramClass);
  if (hologramData == nullptr) {
    return;
  }
  hologramData->InheritBuildModes = false;
  hologramData->BuildModes.Empty();
}

void ABGC_AbstractPlayerSubsystem::RemoveBuildModeDataEntry(
  TSubclassOf<AFGHologram> HologramClass, TSubclassOf<UFGBuildGunModeDescriptor> BuildModeClass) {
  auto hologramData = BuildModesData.Find(HologramClass);
  if (hologramData == nullptr) {
    return;
  }
  hologramData->BuildModes.Remove(BuildModeClass);
}

void ABGC_AbstractPlayerSubsystem::SetBuildModeDataChecked(
  TSubclassOf<AFGHologram> HologramClass, const FBGC_BuildMode_Data& HologramBuildModeData) {
  auto& hologramData = BuildModesData.FindChecked(HologramClass);
  hologramData = HologramBuildModeData;
}

void ABGC_AbstractPlayerSubsystem::SetBuildModeDataEntryChecked(
  TSubclassOf<AFGHologram> HologramClass,
  TSubclassOf<UFGBuildGunModeDescriptor> BuildModeClass,
  const FBGC_BuildMode_DataEntry& HologramBuildModeDataEntry) {
  auto& hologramData = BuildModesData.FindChecked(HologramClass);
  hologramData.BuildModes.Add(BuildModeClass, HologramBuildModeDataEntry);
}

void ABGC_AbstractPlayerSubsystem::RebuildBuildModesData() {
  if (!IsValid(RecipeManager)) {
    RecipeManager = AFGRecipeManager::Get(this);
  }

  TMap<TSubclassOf<AFGHologram>, TArray<TSubclassOf<UFGBuildingDescriptor>>> hologramToBuildingDescriptorsMap;
  TMap<TSubclassOf<AFGHologram>, TArray<TSubclassOf<UFGBuildingDescriptor>>> aliasedHologramToBuildingDescriptorsMap;

  TArray<UClass*> buildingDescriptorsUClass;
  GetDerivedClasses(UFGBuildingDescriptor::StaticClass(), buildingDescriptorsUClass, true);

  for (auto buildingDescriptorUClass : buildingDescriptorsUClass) {
    auto buildingDescriptor = (TSubclassOf<UFGBuildingDescriptor>)buildingDescriptorUClass;
    auto hologramClass = UFGBuildDescriptor::GetHologramClass(*buildingDescriptor);
    if (hologramClass == nullptr) {
      continue;
    }
    if (!IsValid(RecipeManager) || RecipeManager->FindRecipesByProduct(buildingDescriptor, true).Num() == 0) {
      continue;
    }

    hologramToBuildingDescriptorsMap.FindOrAdd(hologramClass).AddUnique(*buildingDescriptor);
  }

  for (auto& pair : hologramToBuildingDescriptorsMap) {
    auto& hologramClass = pair.Key;
    auto hologram = hologramClass.GetDefaultObject();
    if (hologram == nullptr) {
      continue;
    }

    TArray<TSubclassOf<UFGBuildGunModeDescriptor>> supportedBuildModes;
    hologram->GetSupportedBuildModes_Implementation(supportedBuildModes);
    if (supportedBuildModes.Num() <= 1) {
      continue;
    }

    auto& buildingDescriptors = pair.Value;
    if (buildingDescriptors.Num() == 0) {
      continue;
    }
    buildingDescriptors.StableSort(
      [](auto& A, auto& B) { return UFGItemDescriptor::GetMenuPriority(A) < UFGItemDescriptor::GetMenuPriority(B); });

    // Add valid aliases to the aliases map for handling later.
    if (BuildModesAliases.Contains(hologramClass)) {
      auto& aliasClass = BuildModesAliases.FindChecked(hologramClass);
      auto aliasHologram = aliasClass.GetDefaultObject();
      if (aliasHologram == nullptr) {
        continue;
      }

      TArray<TSubclassOf<UFGBuildGunModeDescriptor>> supportedAliasBuildModes;
      aliasHologram->GetSupportedBuildModes_Implementation(supportedAliasBuildModes);

      if (supportedAliasBuildModes != supportedBuildModes) {
        UE_LOG(
          LogBuildGunConfig,
          Warning,
          TEXT("Alias %s has different supported build modes than its target %s. Skipping alias."),
          *aliasClass->GetName(),
          *hologramClass->GetName());
        continue;
      }

      auto& aliasedBuildingDescriptors = aliasedHologramToBuildingDescriptorsMap.FindOrAdd(hologramClass);
      for (auto buildingDescriptor : buildingDescriptors) {
        aliasedBuildingDescriptors.AddUnique(buildingDescriptor);
      }
      continue;
    }

    auto& buildModeData = BuildModesData.FindOrAdd(hologramClass);
    if (!buildModeData.IsEnabled) {
      continue;
    }
    if (buildModeData.DisplayName.IsEmpty()) {
      UE_LOG(LogBuildGunConfig, Verbose, TEXT("Initializing %s display name."), *hologramClass->GetName());
      buildModeData.DisplayName = UFGItemDescriptor::GetItemName(*buildingDescriptors[0]);
    }
    if (!IsValid(buildModeData.Icon)) {
      buildModeData.Icon = UFGItemDescriptor::GetBigIcon(*buildingDescriptors[0]);
    }

    buildModeData.AppliesTo = buildingDescriptors;

    for (auto index = 0; index < supportedBuildModes.Num(); index++) {
      auto buildModeClass = supportedBuildModes[index];
      auto buildModeDataEntry = buildModeData.BuildModes.Find(buildModeClass);
      if (buildModeDataEntry == nullptr) {
        FBGC_BuildMode_DataEntry newEntry;
        newEntry.Index = index;
        newEntry.IsEnabled = true;
        buildModeData.BuildModes.Add(buildModeClass, newEntry);
      }
    }
  }

  for (auto& buildingDescriptor : aliasedHologramToBuildingDescriptorsMap) {
    auto& hologramClass = buildingDescriptor.Key;
    auto& buildingDescriptors = buildingDescriptor.Value;
    auto& aliasClass = BuildModesAliases.FindChecked(hologramClass);
    auto& buildModeData = BuildModesData.FindChecked(aliasClass);
    buildModeData.AppliesTo.Append(buildingDescriptors);
  }
}

const FBGC_BuildMode_Data& ABGC_AbstractPlayerSubsystem::ResolveBuildModeInheritance(
  TSubclassOf<AFGHologram> HologramClass, const FBGC_BuildMode_Data& HologramBuildModeData) {
  if (!IsValid(HologramBuildModeData.InheritBuildModesFrom)) {
    return HologramBuildModeData;
  }

  auto inheritedData = BuildModesData.Find(HologramBuildModeData.InheritBuildModesFrom);
  if (inheritedData == nullptr) {
    return HologramBuildModeData;
  }

  return *inheritedData;
}

bool ABGC_AbstractPlayerSubsystem::ResolveBuildModeData(
  TSubclassOf<AFGHologram> HologramClass, FBGC_BuildMode_Data& out_BuildModeData) {
  auto hologramBuildModeData = BuildModesData.Find(HologramClass);
  if (hologramBuildModeData == nullptr) {
    auto aliasClass = BuildModesAliases.Find(HologramClass);
    if (aliasClass == nullptr) {
      return false;
    }
    hologramBuildModeData = BuildModesData.Find(*aliasClass);
    if (hologramBuildModeData == nullptr) {
      return false;
    }
  }
  out_BuildModeData = ResolveBuildModeInheritance(HologramClass, *hologramBuildModeData);
  return true;
}

UConfigPropertyRaw* ABGC_AbstractPlayerSubsystem::GetBuildModesProperty() {
  auto World = GetWorld();
  if (!IsValid(World)) {
    return nullptr;
  }

  auto RootInstance = UBGC_AbstractInstance::Get(World);
  if (!IsValid(RootInstance)) {
    return nullptr;
  }

  if (RootInstance->ModConfigurations.Num() == 0) {
    return nullptr;
  }

  auto ConfigClass = RootInstance->ModConfigurations[0];
  if (!IsValid(ConfigClass)) {
    return nullptr;
  }

  auto RootProperty = URuntimeBlueprintFunctionLibrary::Conv_ModConfigurationToConfigProperty(ConfigClass, World);
  if (!IsValid(RootProperty)) {
    return nullptr;
  }

  auto RootSection = Cast<UConfigPropertySection>(RootProperty);
  if (!IsValid(RootSection)) {
    return nullptr;
  }

  auto BuildModesProperty = RootSection->SectionProperties.Find(TEXT("BuildModes"));
  if (BuildModesProperty == nullptr) {
    return nullptr;
  }

  auto BuildModesRaw = Cast<UConfigPropertyRaw>(*BuildModesProperty);
  if (!IsValid(BuildModesRaw)) {
    return nullptr;
  }

  return BuildModesRaw;
}

void ABGC_AbstractPlayerSubsystem::SaveBuildModeDataToConfig() {
  auto BuildModesProperty = GetBuildModesProperty();
  if (!IsValid(BuildModesProperty)) {
    return;
  }

  BuildModesProperty->SetValue(BuildModesDataToJson());
  BuildModesProperty->MarkDirty();
}

void ABGC_AbstractPlayerSubsystem::LoadBuildModeDataFromConfig() {
  auto BuildModesProperty = GetBuildModesProperty();
  if (!IsValid(BuildModesProperty)) {
    return;
  }

  TSharedPtr<FJsonObject> JsonObject = BuildModesProperty->Value->AsObject();
  if (!JsonObject.IsValid()) {
    return;
  }

  LoadBuildModeDataFromJson(JsonObject);
}

TSharedPtr<FJsonObject> ABGC_AbstractPlayerSubsystem::BuildModesDataToJson() {
  TSharedPtr<FJsonObject> HologramsObject = MakeShareable(new FJsonObject());
  for (const auto& HologramEntry : BuildModesData) {
    if (HologramEntry.Key == nullptr) {
      UE_LOG(LogBuildGunConfig, Warning, TEXT("Skipping null hologram key during serialization."));
      continue;
    }

    HologramsObject->SetObjectField(HologramEntry.Key->GetPathName(), HologramEntry.Value.ToJson());
  }

  return HologramsObject;
}

void ABGC_AbstractPlayerSubsystem::LoadBuildModeDataFromJson(TSharedPtr<FJsonObject> JsonObject) {
  if (!JsonObject.IsValid()) {
    return;
  }

  for (const auto& HologramEntry : JsonObject->Values) {
    auto hologramClass = LoadClass<AFGHologram>(nullptr, *HologramEntry.Key);
    if (hologramClass == nullptr) {
      UE_LOG(LogBuildGunConfig, Warning, TEXT("Failed to load hologram class with path %s"), *HologramEntry.Key);
      continue;
    }

    auto hologramDataObject = HologramEntry.Value->AsObject();
    if (hologramDataObject == nullptr) {
      continue;
    }

    FBGC_BuildMode_Data& hologramData = BuildModesData.FindOrAdd(hologramClass);
    auto loadedHologramData = FBGC_BuildMode_Data::FromJson(hologramDataObject);
    hologramData.InheritBuildModes = loadedHologramData.InheritBuildModes;
    hologramData.BuildModes = loadedHologramData.BuildModes;
  }
}

void ABGC_AbstractPlayerSubsystem::FilterAndSortBuildModes(
  const FBGC_BuildMode_Data& HologramBuildModeData,
  TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& out_BuildModes) const {
  if (out_BuildModes.Num() == 0) {
    return;
  }

  auto firstBuildMode = out_BuildModes[0];

  // Remove disabled build modes.
  out_BuildModes.RemoveAll([&](auto& BuildMode) {
    auto Data = HologramBuildModeData.BuildModes.Find(BuildMode);
    // Keep build modes that we don't have data entries for.
    if (Data == nullptr) {
      return false;
    }
    return !Data->IsEnabled;
  });

  // Make sure we have at least one build mode.
  if (out_BuildModes.Num() == 0) {
    out_BuildModes.Add(firstBuildMode);
    return;
  }

  // Sort the build modes.
  out_BuildModes.StableSort([&](auto& A, auto& B) {
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
    auto inheritedData = BuildModesData.Find(HologramBuildModeData.Value.InheritBuildModesFrom);

    // If the inherited data is not found, it's an error.
    if (inheritedData == nullptr) {
      ValidationResult = EDataValidationResult::Invalid;
      Context.AddError(
        FText::FromString(
          FString::Printf(
            TEXT("Hologram class %s inherits build modes from %s but it was not found in the build mode data."),
            *HologramBuildModeData.Key->GetName(),
            *HologramBuildModeData.Value.InheritBuildModesFrom->GetName())));
      continue;
    }

    if (inheritedData->InheritBuildModesFrom != nullptr) {
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

      // Autofix.
      // HologramBuildModeData.Value.InheritBuildModesFrom = inheritedData->InheritBuildModesFrom;
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
