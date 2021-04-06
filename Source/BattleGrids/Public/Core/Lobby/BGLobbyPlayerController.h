// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BGPlayerController.h"
#include "Core/BGTypes.h"


#include "BGLobbyPlayerController.generated.h"

class UBGLobbyMenu;
class UBGGameInstance;
class ABGLobbyGameStateBase;

/**
 * 
 */
UCLASS()
class BATTLEGRIDS_API ABGLobbyPlayerController : public ABGPlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGLobbyPlayerController|Config")
	UBGGameInstance* GameInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGLobbyPlayerController|Config")
	ABGLobbyGameStateBase* LobbyGameState;
};
