#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "BBW_ReorderList_EntryData.generated.h"

UINTERFACE(MinimalAPI, Blueprintable, BlueprintType, Category = "BuildGunConfig|EntryList|EntryData")
class UBBW_ReorderList_EntryData : public UInterface {
	GENERATED_BODY()
};

class IBBW_ReorderList_EntryData {
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "BuildGunConfig|EntryList|EntryData")
	int32 GetOriginalIndex();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "BuildGunConfig|EntryList|EntryData")
	FText GetDisplayText();
};
