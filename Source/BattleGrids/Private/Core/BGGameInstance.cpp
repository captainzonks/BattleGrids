// © 2021 Matthew Barham. All Rights Reserved.


#include "Core/BGGameInstance.h"
#include "Core/BGGameModeBase.h"
#include "Core/BGSaveGame.h"
#include "Core/BGTypes.h"
#include "Core/Gameplay/BGGameplayGameStateBase.h"
#include "Core/Save/BGPlayerSave.h"
#include "Characters/BGCharacter.h"
#include "UI/BGGameHUD.h"
#include "UI/BGInGamePlayerList.h"
#include "UI/BGLoadingScreen.h"
#include "UI/BGLobbyMenu.h"
#include "UI/BGThinkingPopup.h"
#include "UI/BGMainMenu.h"
#include "UI/BGInGameMenu.h"

#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Actors/BGActor.h"
#include "Components/BGSplineWallComponent.h"
#include "Kismet/GameplayStatics.h"

const static FName SESSION_NAME = TEXT("BattleGrids Session");
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("BattleGrids Server");

void UBGGameInstance::Init()
{
	auto const Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found subsystem %s"), *IOnlineSubsystem::Get()->GetSubsystemName().ToString())
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
				this, &UBGGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
				this, &UBGGameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.
			                  AddUObject(this, &UBGGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UBGGameInstance::OnJoinSessionComplete);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Found no subsystem"))
	}

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &UBGGameInstance::OnNetworkFailure);
	}

	if (UGameplayStatics::DoesSaveGameExist(DefaultPlayerSaveSlotName, 0))
	{
		PlayerSave = Cast<UBGPlayerSave>(UGameplayStatics::LoadGameFromSlot(DefaultPlayerSaveSlotName, 0));
	}

	bLoading = false;
}

void UBGGameInstance::LoadMainMenuWidget()
{
	if (!ensure(MainMenuClass)) return;

	if (!Menu)
	{
		Menu = CreateWidget<UBGMainMenu>(this, MainMenuClass);
	}
	if (!ensure(Menu)) return;

	Menu->Setup();

	Menu->SetMenuInterface(this);

	if (!PlayerSave)
	{
		Menu->SetActiveWidget(2);
	}
}

void UBGGameInstance::LoadLobbyWidget()
{
	if (!ensure(LobbyMenuClass)) return;

	if (!Lobby)
	{
		Lobby = CreateWidget<UBGLobbyMenu>(this, LobbyMenuClass);
	}
	if (!ensure(Lobby)) return;

	Lobby->Setup();

	Lobby->SetMenuInterface(this);
}

void UBGGameInstance::LoadGameHUDWidget()
{
	if (!ensure(GameHUDClass)) return;

	if (!GameHUD)
	{
		GameHUD = CreateWidget<UBGGameHUD>(this, GameHUDClass);
	}
	if (!ensure(GameHUD)) return;

	GameHUD->Setup();

	GameHUD->SetMenuInterface(this);

	LoadInGamePlayerListWidget();
}

void UBGGameInstance::LoadInGamePlayerListWidget()
{
	if (!ensure(InGamePlayerListClass)) return;

	if (!InGamePlayerList)
	{
		InGamePlayerList = CreateWidget<UBGInGamePlayerList>(this, InGamePlayerListClass);
	}
	if (!ensure(InGamePlayerList)) return;

	InGamePlayerList->Setup();

	InGamePlayerList->SetMenuInterface(this);
}

void UBGGameInstance::InGameLoadMenuWidget()
{
	if (!ensure(InGameMenuClass)) return;

	if (!InGameMenu)
	{
		InGameMenu = CreateWidget<UBGInGameMenu>(this, InGameMenuClass);
	}
	if (!ensure(InGameMenu)) return;

	InGameMenu->Setup();

	InGameMenu->SetMenuInterface(this);
}

void UBGGameInstance::ShowLoadingScreen()
{
	if (!ensure(LoadingScreenClass)) return;

	if (!LoadingScreen)
	{
		LoadingScreen = CreateWidget<UBGLoadingScreen>(this, LoadingScreenClass);
	}
	if (!ensure(LoadingScreen)) return;

	UE_LOG(LogTemp, Warning, TEXT("Showing Loading Screen"))
	LoadingScreen->AddToViewport(100);
}

void UBGGameInstance::HideLoadingScreen()
{
	UE_LOG(LogTemp, Warning, TEXT("Hiding Loading Screen"))
	if (LoadingScreen && LoadingScreen->IsInViewport())
	{
		LoadingScreen->RemoveFromViewport();
	}
}

void UBGGameInstance::ShowThinkingPopup()
{
	if (!ensure(ThinkingPopupClass)) return;

	if (!ThinkingPopup)
	{
		ThinkingPopup = CreateWidget<UBGThinkingPopup>(this, ThinkingPopupClass);
	}
	if (!ensure(ThinkingPopup)) return;

	UE_LOG(LogTemp, Warning, TEXT("Showing Thinking Popup"))
	if (!ThinkingPopup->IsInViewport())
	{
		ThinkingPopup->AddToViewport(100);
	}
}

void UBGGameInstance::HideThinkingPopup()
{
	UE_LOG(LogTemp, Warning, TEXT("Hiding Thinking Popup"))
	if (ThinkingPopup && ThinkingPopup->IsInViewport())
	{
		ThinkingPopup->RemoveFromViewport();
	}
}

void UBGGameInstance::Host(FString const& ServerName)
{
	if (SessionInterface.IsValid())
	{
		ServerData.Name = ServerName;
		ServerData.MaxPlayers = 5;

		auto const ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (ExistingSession)
		{
			SessionInterface->DestroySession(SESSION_NAME);
		}
		else
		{
			CreateSession();
		}
	}
}

void UBGGameInstance::Join(int const& Index, FBGServerData const& InServerData)
{
	if (!SessionInterface.IsValid()) return;
	if (!SessionSearch.IsValid()) return;

	if (!ensure(Menu)) return;
	Menu->Teardown();

	ServerData = InServerData;

	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
}


void UBGGameInstance::StartGame()
{
	if (bLoading)
	{
		if (!ensure(Menu)) return;
		Menu->Teardown();
	}
	else
	{
		if (!ensure(Lobby)) return;
		Lobby->Teardown();
	}

	auto Engine = GetEngine();
	if (!ensure(Engine)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Starting Game"));

	auto World = GetWorld();
	if (!ensure(World)) return;

	ShowLoadingScreen();

	World->ServerTravel("/Game/BattleGrids/Levels/DefaultGameMap?listen");
}

void UBGGameInstance::LoadMainMenu()
{
	auto PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController)) return;

	bLoading = false;

	PlayerController->ClientTravel("/Game/BattleGrids/Levels/MainMenu", TRAVEL_Absolute);
}

void UBGGameInstance::RefreshServerList()
{
	ToggleThinkingPopup(true);
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		// UE_LOG(LogTemp, Warning, TEXT("Starting Find Session"))
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UBGGameInstance::RefreshLoadGameList()
{
	ToggleThinkingPopup(true);

	if (UGameplayStatics::DoesSaveGameExist(DefaultGameSaveSlotName, 0))
	{
		TArray<UBGGameSave*> SaveGames;
		UBGGameSave* NewSaveGame = Cast<UBGGameSave>(UGameplayStatics::LoadGameFromSlot(DefaultGameSaveSlotName, 0));
		if (NewSaveGame)
		{
			SaveGames.AddUnique(NewSaveGame);
		}
		Menu->SetSaveGameList(SaveGames);
	}
}

void UBGGameInstance::RefreshPlayerLists(TArray<FBGPlayerInfo> const& InPlayerInfo)
{
	// UE_LOG(LogTemp, Warning, TEXT("Refreshing Player Lists, Length : %d"), InPlayerInfo.Num())

	if (InGamePlayerList)
	{
		InGamePlayerList->SetPlayerList(InPlayerInfo);
	}

	if (Lobby)
	{
		Lobby->SetPlayerList(InPlayerInfo);
	}
}

void UBGGameInstance::ToggleLoadingScreen(bool const bIsLoading)
{
	auto const World = GetWorld();
	if (World && !World->bIsTearingDown)
	{
		bIsLoading ? ShowLoadingScreen() : HideLoadingScreen();
	}
}

void UBGGameInstance::ToggleThinkingPopup(bool const bThinking)
{
	auto const World = GetWorld();
	if (World && !World->bIsTearingDown)
	{
		bThinking ? ShowThinkingPopup() : HideThinkingPopup();
	}
}

// void UBGGameInstance::SavePlayerInfo(FBGPlayerInfo const& InPlayerInfo)
// {
// 	SavePlayerInfo(InPlayerInfo);
// }

void UBGGameInstance::UpdatePlayerInfo(int const& Index, FBGPlayerInfo const& InPlayerInfo)
{
	auto const World = GetWorld();
	if (World)
	{
		auto CastGameMode = World->GetAuthGameMode<ABGGameModeBase>();
		if (CastGameMode)
		{
			CastGameMode->SendUpdatedPlayerInfoToPlayer(Index, InPlayerInfo);
		}
	}
}

void UBGGameInstance::CreateSession() const
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;
		if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
		{
			SessionSettings.bIsLANMatch = true;
		}
		else
		{
			SessionSettings.bIsLANMatch = false;
		}
		// Max Players set to 5 by default; this should be adjustable by the host, however
		SessionSettings.NumPublicConnections = 5;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, ServerData.Name,
		                    EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
	}
}

void UBGGameInstance::SavePlayerInfo(FBGPlayerInfo const& InPlayerInfo)
{
	if (!PlayerSave->IsValidLowLevel())
	{
		PlayerSave = Cast<UBGPlayerSave>(UGameplayStatics::CreateSaveGameObject(UBGPlayerSave::StaticClass()));
		if (!ensure(PlayerSave)) return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Saving Player Info"))
	PlayerSave->Save(InPlayerInfo);
	OnSaveGame.Broadcast(InPlayerInfo);
	UGameplayStatics::SaveGameToSlot(PlayerSave, DefaultPlayerSaveSlotName, 0);
}

void UBGGameInstance::SaveGameInfo()
{
	auto const World = GetWorld();

	if (!GameSave->IsValidLowLevel())
	{
		GameSave = Cast<UBGGameSave>(UGameplayStatics::CreateSaveGameObject(UBGGameSave::StaticClass()));
		if (!ensure(GameSave)) return;
	}

	if (World)
	{
		auto GameState = World->GetGameState<ABGGameplayGameStateBase>();
		if (GameState)
		{
			auto SpawnedCharacters = GameState->GetSpawnedCharacters();

			for (auto Character : SpawnedCharacters)
			{
				Character->UpdateCharacterSaveLocation();
				GameSave->SaveCharacterModelInfo(Character->GetCharacterModelSaveInfo());
			}

			auto SpawnedSplineWallActors = GameState->GetSpawnedActorsOfType(EBGActorType::Structure);

			for (auto Structure : SpawnedSplineWallActors)
			{
				auto SplineWallComponent = Cast<UBGSplineWallComponent>(Structure->GetComponentByClass(
					UBGSplineWallComponent::StaticClass()));
				if (SplineWallComponent)
				{
					SplineWallComponent->UpdateWallSplineSaveInfo();
					GameSave->SaveWallSplineInfo(SplineWallComponent->GetWallSplineSaveInfo());
				}
			}

			UGameplayStatics::SaveGameToSlot(GameSave, DefaultGameSaveSlotName, 0);
		}
	}
}

bool UBGGameInstance::LoadPlayerInfo(FBGPlayerInfo& OutPlayerInfo)
{
	PlayerSave = Cast<UBGPlayerSave>(UGameplayStatics::LoadGameFromSlot(DefaultPlayerSaveSlotName, 0));
	if (PlayerSave->IsValidLowLevel())
	{
		OutPlayerInfo = PlayerSave->GetPlayerInfo();
		return true;
	}
	return false;
}

void UBGGameInstance::LoadGameInfo(int const& Index, UBGGameSave* InGameSave)
{
	if (InGameSave->IsValidLowLevel())
	{
		GameSave = InGameSave;
		bLoading = true;

		StartGame();
	}
}

void UBGGameInstance::OnCreateSessionComplete(FName const SessionName, bool bSuccess)
{
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not create session"))
		return;
	}

	if (!ensure(Menu)) return;
	Menu->Teardown();

	auto Engine = GetEngine();
	if (!ensure(Engine)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, (TEXT("Hosting %s"), ServerData.Name));

	auto World = GetWorld();
	if (!ensure(World)) return;

	ToggleLoadingScreen(true);

	World->ServerTravel("/Game/BattleGrids/Levels/Lobby?listen");
}

void UBGGameInstance::OnDestroySessionComplete(FName const SessionName, bool bSuccess)
{
	if (bSuccess)
	{
		CreateSession();
		UE_LOG(LogTemp, Warning, TEXT("Destroyed existing session"))
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not destroy existing session"))
	}
}

void UBGGameInstance::OnFindSessionsComplete(bool bSuccess)
{
	if (bSuccess && SessionSearch.IsValid() && Menu)
	{
		UE_LOG(LogTemp, Warning, TEXT("Finished Find Session"))

		TArray<FBGServerData> ServerNames;
		for (FOnlineSessionSearchResult const& Result : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found session names: %s"), *Result.GetSessionIdStr())
			FBGServerData Data;
			Data.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			Data.CurrentPlayers = Data.MaxPlayers - Result.Session.NumOpenPublicConnections - Result.Session.
				NumOpenPrivateConnections;
			UE_LOG(LogTemp, Warning, TEXT("Open public connections: %d"), Result.Session.NumOpenPublicConnections)
			Data.HostUsername = Result.Session.OwningUserName;
			FString ServerName;
			if (Result.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
			{
				Data.Name = ServerName;
			}
			else
			{
				Data.Name = "Could not find name.";
			}
			ServerNames.Add(Data);
		}

		Menu->SetServerList(ServerNames);
		ToggleThinkingPopup(false);
	}
}

void UBGGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid()) return;

	FString Address;
	if (!SessionInterface->GetResolvedConnectString(SessionName, Address))
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not get connect string."))
		return;
	}

	auto const Engine = GetEngine();
	if (!ensure(Engine)) return;

	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

	auto PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController)) return;

	ToggleLoadingScreen(true);

	PlayerController->ClientTravel(Address, TRAVEL_Absolute);
}

void UBGGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
                                       const FString& ErrorString)
{
	LoadMainMenu();
}
