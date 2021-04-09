// © 2021 Matthew Barham. All Rights Reserved.

#include "Core/BGGameModeBase.h"

#include "Core/BGPlayerState.h"
#include "Core/BGPlayerController.h"
#include "Core/Lobby/BGLobbyGameStateBase.h"
#include "Kismet/GameplayStatics.h"

void ABGGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

void ABGGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	SetUpdateTimer();
}

void ABGGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	auto const CastPlayerController = Cast<APlayerController>(Exiting);
	auto const PlayerInfoToRemove = CastPlayerController->GetPlayerState<ABGPlayerState>()->GetPlayerInfo();
	if (CastPlayerController)
	{
		RemovePlayerInfoFromArray(PlayerInfoToRemove);
	}
}

void ABGGameModeBase::UpdatePlayerInfoInArray(FBGPlayerInfo const& InPlayerInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("ConnectedPlayersInfo Array Length: %d"), ConnectedPlayersInfo.Num())

	int Index{};
	if (ConnectedPlayersInfo.Find(InPlayerInfo, Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("Found Existing Player Info"))
		ConnectedPlayersInfo[Index] = InPlayerInfo;
	}
	else
	{
		AddPlayerInfoToArray(InPlayerInfo);
	}

	SetUpdateTimer();
}

void ABGGameModeBase::AddPlayerInfoToArray(FBGPlayerInfo const& InPlayerInfo)
{
	ConnectedPlayersInfo.AddUnique(InPlayerInfo);
	UE_LOG(LogTemp, Warning, TEXT("Adding Player Info to Array"))
}

void ABGGameModeBase::SetUpdateTimer()
{
	auto const World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
		World->GetTimerManager().SetTimer(UpdateTimer, this, &ABGGameModeBase::UpdateConnectedPlayersUI, 1.5f, false);
	}
}

void ABGGameModeBase::RemovePlayerInfoFromArray(FBGPlayerInfo const& InPlayerInfo)
{
	ConnectedPlayersInfo.Remove(InPlayerInfo);
	UE_LOG(LogTemp, Warning, TEXT("Removing Player Info from Array"))

	SetUpdateTimer();
}

void ABGGameModeBase::UpdateConnectedPlayersUI()
{
	auto const World = GetWorld();
	if (World)
	{
		for (auto Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if (auto PlayerController = Cast<ABGPlayerController>(Iterator->Get()))
			{
				PlayerController->UpdateUI(ConnectedPlayersInfo);
			}
		}

		World->GetTimerManager().ClearAllTimersForObject(this);
		World->GetTimerManager().SetTimer(UpdateTimer, this, &ABGGameModeBase::UpdateConnectedPlayersUI, 10.f, true);
	}
}
