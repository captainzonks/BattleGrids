// © 2021 Matthew Barham. All Rights Reserved.


#include "Core/Lobby/BGLobbyPlayerController.h"

#include "Core/BGGameInstance.h"
#include "Core/BGPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "UI/BGLobbyMenu.h"

void ABGLobbyPlayerController::UpdateLobbyInformation_Implementation()
{
	auto const GameInstance = Cast<UBGGameInstance>(GetGameInstance());
	if (GameInstance && GameInstance->GetLobby())
	{
		UE_LOG(LogTemp, Warning, TEXT("Current Connected Players: %d"), ServerData.CurrentPlayers)
		GameInstance->GetLobby()->UpdateLobbyInfo(ServerData);
	}
}

void ABGLobbyPlayerController::UpdateServerData(FBGServerData const& InServerData)
{
	ServerData = InServerData;

	UpdateLobbyInformation();
}

void ABGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	auto const GameInstance = Cast<UBGGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		GameInstance->LoadLobbyWidget();
		ServerData = GameInstance->GetInitialServerData();
	}
}

void ABGLobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGLobbyPlayerController, ServerData)
}
