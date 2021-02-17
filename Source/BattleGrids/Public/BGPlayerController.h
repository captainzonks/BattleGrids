// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "BGTypes.h"
#include "GameFramework/PlayerController.h"
#include "BGPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEGRIDS_API ABGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ABGPlayerController();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGPlayerController|Config")
	FBGPlayerInfo PlayerInfo;
};
