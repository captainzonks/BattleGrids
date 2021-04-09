// © 2021 Matthew Barham. All Rights Reserved.


#include "Core/Lobby/BGLobbyPlayerController.h"

#include "Core/BGGameInstance.h"
#include "Core/BGPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "UI/BGLobbyMenu.h"

ABGLobbyPlayerController::ABGLobbyPlayerController()
{
}

void ABGLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetupLobbyUI();
}

void ABGLobbyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABGLobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ABGLobbyPlayerController::SetupLobbyUI_Implementation()
{
	auto GameInstance = GetGameInstance<UBGGameInstance>();
	if (GameInstance)
	{
		GameInstance->ShowLoadingScreen();
		GameInstance->LoadLobbyWidget();
		GameInstance->HideLoadingScreen();
	}
}