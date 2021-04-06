// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerController.h"
#include "BGPlayerController.generated.h"

class UBGGameInstance;

/**
 * Parent Player Controller class for BattleGrids
 */
UCLASS()
class BATTLEGRIDS_API ABGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ABGPlayerController();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	uint8 bLoading : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGPlayerController|Config")
	UBGGameInstance* GameInstance;
};
