#include "BGC_AbstractWorld.h"

#include "CoreMinimal.h"

#include "FGBuildGunModeDescriptor.h"
#include "Equipment/FGBuildGunBuild.h"

#include "Module/WorldModuleManager.h"
#include "Patching/NativeHookManager.h"

#include "BGC_AbstractInstance.h"
#include "BGC_AbstractPlayerSubsystem.h"
#include "BGC_Utils.h"
#include "UserSettings/BGC_BuildModeGroup.h"

UBGC_AbstractWorld* UBGC_AbstractWorld::Get(const UObject* WorldContext) {
	if (!ensureMsgf(IsValid(WorldContext), TEXT("Invalid world context."))) {
		return nullptr;
	}

	const auto World = WorldContext->GetWorld();
	if (!ensureMsgf(IsValid(World), TEXT("Failed to get the world."))) {
		return nullptr;
	}

	const auto WorldModuleManager = World->GetSubsystem<UWorldModuleManager>();
	if (!ensureMsgf(IsValid(WorldModuleManager), TEXT("Failed to find world module manager."))) {
		return nullptr;
	}

	const auto WorldModule = WorldModuleManager->FindModule(UBGC_AbstractInstance::ModName);
	if (!ensureMsgf(IsValid(WorldModule), TEXT("Failed to find world module."))) {
		return nullptr;
	}

	return Cast<UBGC_AbstractWorld>(WorldModule);
}

void UBGC_AbstractWorld::DispatchLifecycleEvent(const ELifecyclePhase Phase) {
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
		default: {}
	}
}

void UBGC_AbstractWorld::SubscribeToHooks() {
	const auto BuildGunStateBuild = GetMutableDefault<UFGBuildGunStateBuild>();

	/**
	 * Filter out disabled build modes and sort the rest according to our config data.
	 */
	SUBSCRIBE_METHOD_VIRTUAL_AFTER(
		UFGBuildGunStateBuild::GetSupportedBuildModes_Implementation,
		BuildGunStateBuild,
		[this](const UFGBuildGunStateBuild* Self, TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& Out_BuildModes) {
			if (!IsValid(PlayerSubsystem) || Out_BuildModes.Num() <= 1) {
				return;
			}

			const auto HologramClass = UBGC_Utils::GetHologramClass(Self);
			if (!IsValid(HologramClass)) {
				return;
			}

			const auto BuildModeGroup = PlayerSubsystem->FindBuildModeGroupOf(HologramClass);
			if (!BuildModeGroup.IsValid() || !BuildModeGroup->IsEnabled) {
				return;
			}

			PlayerSubsystem->FilterAndSortBuildModes(BuildModeGroup.ToSharedRef(), Out_BuildModes);
		}
	);

	/**
	 * Use the first build mode as the initial build mode.
	 */
	SUBSCRIBE_METHOD_VIRTUAL(
		UFGBuildGunStateBuild::GetInitialBuildGunMode_Implementation,
		BuildGunStateBuild,
		[this](auto& Scope, const UFGBuildGunStateBuild* Self) {
			TSubclassOf<UFGBuildGunModeDescriptor> Result = Scope(Self);
			TArray<TSubclassOf<UFGBuildGunModeDescriptor>> SupportedBuildModes;
			Self->GetSupportedBuildModes_Implementation(SupportedBuildModes);
			if (SupportedBuildModes.Num() > 0) {
				Scope.Override(SupportedBuildModes[0]);
				return;
			}
			Scope.Override(Result);
		}
	);
}
