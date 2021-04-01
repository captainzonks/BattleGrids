// © 2021 Matthew Barham. All Rights Reserved.


#include "Core/Lobby/BGLobbyPlayerController.h"

#include "Blueprint/UserWidget.h"

void ABGLobbyPlayerController::Setup()
{
	if(!ensure(LobbyWidgetClass)) return;

	LobbyWidget = CreateWidget<UBGLobbyWidget>(this, LobbyWidgetClass);

	// TODO: Setup the lobby widget, and move Blueprint code over to C++
}
