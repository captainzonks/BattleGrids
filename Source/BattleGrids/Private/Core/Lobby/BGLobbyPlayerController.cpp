// © 2021 Matthew Barham. All Rights Reserved.


#include "Core/Lobby/BGLobbyPlayerController.h"

#include "Core/BGGameInstance.h"
#include "Core/BGPlayerState.h"
#include "Core/Lobby/BGLobbyGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "UI/BGLobbyMenu.h"

void ABGLobbyPlayerController::ServerRefreshLobbyPlayerList_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Server: Refresh Lobby Player List!"))
	if (LobbyGameState)
	{
		MulticastUpdateConnectedPlayersLobby(LobbyGameState->GetConnectedPlayersInfo());
	}
}

void ABGLobbyPlayerController::MulticastUpdateConnectedPlayersLobby_Implementation(
	TArray<FBGPlayerInfo> const& InConnectedPlayersInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("Multicast Refresh Lobby Player List!"))
	GetGameInstance<UBGGameInstance>()->RefreshConnectedPlayersList(InConnectedPlayersInfo);
	bLobbyNeedsUpdating = false;
}

void ABGLobbyPlayerController::SetLobbyNeedsUpdating(bool const bInValue)
{
	bLobbyNeedsUpdating = bInValue;
}

void ABGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bLobbyNeedsUpdating = false;
}

void ABGLobbyPlayerController::Tick(float DeltaSeconds)
{
	if (bLoading && LobbyGameState)
	{
		GameInstance->LoadLobbyWidget();
		GameInstance->HideLoadingScreen();
		bLoading = false;
		Super::Tick(DeltaSeconds);
	}
	else if (bLoading && !LobbyGameState)
	{
		LobbyGameState = GetWorld()->GetGameState<ABGLobbyGameStateBase>();
	}

	if (bLobbyNeedsUpdating)
	{
		ServerRefreshLobbyPlayerList();
	}
}

void ABGLobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGLobbyPlayerController, bLobbyNeedsUpdating)
}
