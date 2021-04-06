// © 2021 Matthew Barham. All Rights Reserved.


#include "Core/Lobby/BGLobbyPlayerController.h"

#include "Core/BGGameInstance.h"
#include "Core/BGPlayerState.h"
#include "Core/Lobby/BGLobbyGameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "UI/BGLobbyMenu.h"

void ABGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	while (!LobbyGameState)
	{
		LobbyGameState = GetWorld()->GetGameState<ABGLobbyGameStateBase>();
	}

    GameInstance = Cast<UBGGameInstance>(GetGameInstance());
	if (GameInstance && LobbyGameState)
	{
		GameInstance->LoadLobbyWidget();
		GameInstance->HideLoadingScreen();
	}
}

void ABGLobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}
