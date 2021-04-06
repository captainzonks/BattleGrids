// © 2021 Matthew Barham. All Rights Reserved.


#include "Core/BGPlayerController.h"

#include "Core/BGGameInstance.h"
#include "Core/BGPlayerState.h"
#include "Engine/DemoNetDriver.h"

ABGPlayerController::ABGPlayerController()
{
	bReplicates = true;
}

void ABGPlayerController::BeginPlay()
{
	Super::BeginPlay();
}
