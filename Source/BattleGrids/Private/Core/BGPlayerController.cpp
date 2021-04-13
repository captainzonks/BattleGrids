// © 2021 Matthew Barham. All Rights Reserved.

#include "Core/BGPlayerController.h"

#include "Core/BGGameInstance.h"
#include "Core/BGGameModeBase.h"
#include "Net/UnrealNetwork.h"

void ABGPlayerController::ClientUpdateUI_Implementation(TArray<FBGPlayerInfo> const& InPlayerInfoArray)
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
	
	ServerBindDelegates();
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

void ABGPlayerController::ClientToggleThinkingPopup_Implementation(bool const bIsThinking)
{
	GetGameInstance<UBGGameInstance>()->ToggleThinkingPopup(bIsThinking);
}

void ABGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGPlayerController, bLoading)
}

void ABGPlayerController::ServerBindDelegates_Implementation()
{
	auto const World = GetWorld();
	if (World)
	{
		World->GetAuthGameMode<ABGGameModeBase>()->OnServerThinking.AddUniqueDynamic(
			this, &ABGPlayerController::ClientToggleThinkingPopup);
	}
}
