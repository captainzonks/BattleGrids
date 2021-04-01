// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BGPlayerController.h"
#include "BGLobbyPlayerController.generated.h"

class UBGLobbyWidget;

/**
 * 
 */
UCLASS()
class BATTLEGRIDS_API ABGLobbyPlayerController : public ABGPlayerController
{
	GENERATED_BODY()

public:

	void Setup();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGLobbyPlayerController|Config")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGLobbyPlayerController|Config")
	UBGLobbyWidget* LobbyWidget;
};
