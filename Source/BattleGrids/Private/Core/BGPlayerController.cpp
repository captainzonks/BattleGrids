// © 2021 Matthew Barham. All Rights Reserved.

#include "Core/BGPlayerController.h"

#include "Core/BGGameInstance.h"
#include "Net/UnrealNetwork.h"

void ABGPlayerController::UpdateUI_Implementation(TArray<FBGPlayerInfo> const& InPlayerInfoArray)
{
	auto GameInstance = GetGameInstance<UBGGameInstance>();
	if (GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController: Updating UI, length: %d"), InPlayerInfoArray.Num())
		GameInstance->RefreshPlayerLists(InPlayerInfoArray);
	}
}

ABGPlayerController::ABGPlayerController()
{
	bLoading = true;
}

void ABGPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ABGPlayerController::ServerToggleLoadingState_Implementation(bool const bIsLoading)
{
	bLoading = bIsLoading;
}

void ABGPlayerController::ToggleLoadingState(bool const bIsLoading)
{
	if (GetNetMode() == NM_Client)
	{
		bLoading = bIsLoading;
	}

	ServerToggleLoadingState(bIsLoading);
}

void ABGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGPlayerController, bLoading)
}
