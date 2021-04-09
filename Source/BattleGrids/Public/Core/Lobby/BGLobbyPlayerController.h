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
 * Player Controller for Lobby
 */
UCLASS()
class BATTLEGRIDS_API ABGLobbyPlayerController : public ABGPlayerController
{
	GENERATED_BODY()

public:
	ABGLobbyPlayerController();

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "BGLobbyPlayerController|UI")
	void SetupLobbyUI();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};