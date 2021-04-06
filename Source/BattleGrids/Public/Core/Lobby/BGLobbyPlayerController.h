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

public:

	void SetLobbyNeedsUpdating(bool const bInValue);

	// Network Functions
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGLobbyPlayerController|Network")
	void ServerRefreshLobbyPlayerList();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "BGLobbyPlayerController|Network")
	void MulticastUpdateConnectedPlayersLobby(TArray<FBGPlayerInfo> const& InConnectedPlayersInfo);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGLobbyPlayerController|Config")
	ABGLobbyGameStateBase* LobbyGameState;

	UPROPERTY(Replicated)
	uint8 bLobbyNeedsUpdating : 1;
};
