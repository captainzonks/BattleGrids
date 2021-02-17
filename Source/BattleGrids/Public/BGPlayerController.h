// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "BGTypes.h"
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGPlayerController|Config")
	FBGPlayerInfo PlayerInfo;
};
