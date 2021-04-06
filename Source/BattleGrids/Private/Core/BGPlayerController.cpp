// © 2021 Matthew Barham. All Rights Reserved.


#include "Core/BGPlayerController.h"
#include "Core/BGGameInstance.h"

#include "Engine/DemoNetDriver.h"

ABGPlayerController::ABGPlayerController()
{
	bReplicates = true;
	bLoading = true;
}

void ABGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	GameInstance = GetGameInstance<UBGGameInstance>();
}

void ABGPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (bLoading && GameInstance)
	{
		GameInstance->ShowLoadingScreen();
		if (GetWorld()->GetGameState() && PlayerState)
		{
			GameInstance->HideLoadingScreen();
			bLoading = false;
		}
	}
}