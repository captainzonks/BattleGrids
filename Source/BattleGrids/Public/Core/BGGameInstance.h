// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BGGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEGRIDS_API UBGGameInstance : public UGameInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	class UBGSaveGame* SaveGame;
};
