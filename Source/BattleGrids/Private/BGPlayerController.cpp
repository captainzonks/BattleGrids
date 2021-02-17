// © 2021 Matthew Barham. All Rights Reserved.


#include "BGPlayerController.h"

#include "Engine/DemoNetDriver.h"

ABGPlayerController::ABGPlayerController()
{
	bReplicates = true;
}

void ABGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGPlayerController, PlayerInfo)
}

