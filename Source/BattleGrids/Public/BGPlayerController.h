// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerController.h"
#include "BGPlayerController.generated.h"

/**
 * Parent Player Controller class for BattleGrids
 */
UCLASS()
class BATTLEGRIDS_API ABGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ABGPlayerController();
};
