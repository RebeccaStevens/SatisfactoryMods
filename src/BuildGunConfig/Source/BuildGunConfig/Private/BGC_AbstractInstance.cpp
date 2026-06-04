#include "BGC_AbstractInstance.h"

#include "CoreMinimal.h"

#include "Module/GameInstanceModuleManager.h"

#include "BGC_Module.h"

UBGC_AbstractInstance* UBGC_AbstractInstance::Get(UObject* WorldContext) {
  if (!ensureMsgf(IsValid(WorldContext), TEXT("Invalid world context."))) {
    return nullptr;
  }

  UWorld* World = WorldContext->GetWorld();
  if (!ensureMsgf(IsValid(World), TEXT("Failed to get the world."))) {
    return nullptr;
  }

  UGameInstance* GameInstance = World->GetGameInstance();
  if (!ensureMsgf(IsValid(GameInstance), TEXT("Failed to get the game instance."))) {
    return nullptr;
  }

  UGameInstanceModuleManager* ModuleManager = GameInstance->GetSubsystem<UGameInstanceModuleManager>();
  if (!ensureMsgf(IsValid(ModuleManager), TEXT("Failed to find game instance module manager."))) {
    return nullptr;
  }

  UGameInstanceModule* InstanceMod = ModuleManager->FindModule(UBGC_AbstractInstance::ModName);
  if (!ensureMsgf(IsValid(InstanceMod), TEXT("Failed to find game instance module."))) {
    return nullptr;
  }

  UBGC_AbstractInstance* Instance = Cast<UBGC_AbstractInstance>(InstanceMod);
  if (!ensureMsgf(IsValid(Instance), TEXT("Failed to cast game instance module to UBGC_AbstractInstance."))) {
    return nullptr;
  }

  return Instance;
}

void UBGC_AbstractInstance::DispatchLifecycleEvent(ELifecyclePhase Phase) {
  Super::DispatchLifecycleEvent(Phase);
  switch (Phase) {
    case ELifecyclePhase::CONSTRUCTION: {
      return;
    }
    case ELifecyclePhase::INITIALIZATION: {
      return;
    }
    case ELifecyclePhase::POST_INITIALIZATION: {
      return;
    }
  }
}
