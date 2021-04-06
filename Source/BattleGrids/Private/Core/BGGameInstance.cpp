// © 2021 Matthew Barham. All Rights Reserved.


#include "Core/BGGameInstance.h"
#include "UI/BGMainMenu.h"
#include "UI/BGInGameMenu.h"

#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Core/BGTypes.h"
#include "Core/Lobby/BGLobbyGameStateBase.h"
#include "UI/BGLoadingScreen.h"
#include "UI/BGLobbyMenu.h"

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
}

void UBGGameInstance::LoadMainMenuWidget()
{
	if (!ensure(MainMenuClass)) return;

	Menu = CreateWidget<UBGMainMenu>(this, MainMenuClass);
	if (!ensure(Menu)) return;

	Menu->Setup();

	Menu->SetMenuInterface(this);
}

void UBGGameInstance::LoadLobbyWidget()
{
	ShowLoadingScreen();
	
	if (!ensure(LobbyMenuClass)) return;

	if (!Lobby)
	{
		Lobby = CreateWidget<UBGLobbyMenu>(this, LobbyMenuClass);
	}
	if (!ensure(Lobby)) return;

	Lobby->Setup();

	Lobby->SetMenuInterface(this);
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
		LoadingScreen = CreateWidget<UBGLoadingScreen>(this, LoadingScreenClass);\
	}
	if (!ensure(LoadingScreen)) return;

	LoadingScreen->AddToViewport(100);
}

void UBGGameInstance::HideLoadingScreen()
{
	UE_LOG(LogTemp, Warning, TEXT("Hiding loading screen"))
	if (LoadingScreen && LoadingScreen->IsInViewport())
	{
		LoadingScreen->RemoveFromViewport();
	}
}

void UBGGameInstance::Host(FString const& ServerName)
{
	if (SessionInterface.IsValid())
	{
		ServerData.Name = ServerName;
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

void UBGGameInstance::Join(uint32 const& Index, FBGServerData const& InServerData)
{
	if (!SessionInterface.IsValid()) return;
	if (!SessionSearch.IsValid()) return;

	if (Menu)
	{
		Menu->Teardown();
	}

	ServerData = InServerData;

	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
}

void UBGGameInstance::LoadMainMenu()
{
	auto PlayerController = GetFirstLocalPlayerController();
	if (!ensure(PlayerController)) return;

	PlayerController->ClientTravel("/Game/BattleGrids/Levels/MainMenu", TRAVEL_Absolute);
}

void UBGGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		UE_LOG(LogTemp, Warning, TEXT("Starting Find Session"))
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UBGGameInstance::RefreshConnectedPlayersList(TArray<FBGPlayerInfo> const& InPlayerInfo)
{
	if (Lobby)
	{
		Lobby->SetPlayerList(InPlayerInfo);
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
		SessionSettings.NumPublicConnections = 5;
		SessionSettings.bShouldAdvertise = true;
		SessionSettings.bUsesPresence = true;
		SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, ServerData.Name,
		                    EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
	}
}

void UBGGameInstance::OnCreateSessionComplete(FName const SessionName, bool bSuccess)
{
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not create session"))
		return;
	}

	if (Menu)
	{
		Menu->Teardown();
	}

	auto Engine = GetEngine();
	if (!ensure(Engine)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, (TEXT("Hosting %s"), ServerData.Name));

	auto World = GetWorld();
	if (!ensure(World)) return;

	ShowLoadingScreen();

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

	ShowLoadingScreen();

	PlayerController->ClientTravel(Address, TRAVEL_Absolute);
}

void UBGGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
                                       const FString& ErrorString)
{
	LoadMainMenu();
}
