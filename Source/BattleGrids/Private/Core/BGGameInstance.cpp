// © 2021 Matthew Barham. All Rights Reserved.


#include "Core/BGGameInstance.h"
#include "UI/BGMainMenu.h"
#include "UI/BGInGameMenu.h"

#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

const static FName SESSION_NAME = TEXT("Test Session");

void UBGGameInstance::Init()
{
	Super::Init();

	if (MenuClass->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("Found class %s"), *MenuClass->GetName());
	}

	if (InGameMenuClass->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("Found class %s"), *InGameMenuClass->GetName());
	}

	auto const Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found subsystem %s"), *IOnlineSubsystem::Get()->GetSubsystemName().ToString())
		SessionInterface = Subsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Found session interface"))
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
				this, &UBGGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
				this, &UBGGameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.
			                  AddUObject(this, &UBGGameInstance::OnFindSessionsComplete);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Found no subsystem"))
	}
}

void UBGGameInstance::LoadMenu()
{
	if (!ensure(MenuClass)) return;
	Menu = CreateWidget<UBGMainMenu>(this, MenuClass);
	if (!ensure(Menu)) return;

	Menu->Setup();

	Menu->SetMenuInterface(this);
}

void UBGGameInstance::InGameLoadMenu()
{
	if (!ensure(InGameMenuClass)) return;
	InGameMenu = CreateWidget<UBGInGameMenu>(this, InGameMenuClass);
	if (!ensure(InGameMenu)) return;

	InGameMenu->Setup();

	InGameMenu->SetMenuInterface(this);
}

void UBGGameInstance::Host()
{
	if (Menu)
	{
		Menu->Teardown();
	}

	if (SessionInterface.IsValid())
	{
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

void UBGGameInstance::Join(FString const& Address)
{
	if (Menu)
	{
		Menu->SetServerList({"Test1", "Test2", "Test3"});

		// Menu->Teardown();
	}
	//
	// auto const Engine = GetEngine();
	// if (ensure(!Engine)) return;
	//
	// Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
	//
	// auto PlayerController = GetFirstLocalPlayerController();
	// if (!ensure(PlayerController)) return;
	//
	// PlayerController->ClientTravel(Address, TRAVEL_Absolute);
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
		// SessionSearch->bIsLanQuery = true;
		UE_LOG(LogTemp, Warning, TEXT("Starting Find Session"))
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UBGGameInstance::CreateSession() const
{
	if (SessionInterface.IsValid())
	{
		FOnlineSessionSettings SessionSettings;
		SessionSettings.bIsLANMatch = true;
		SessionSettings.NumPublicConnections = 2;
		SessionSettings.bShouldAdvertise = true;

		SessionInterface->CreateSession(0, SESSION_NAME, SessionSettings);
	}
}

void UBGGameInstance::OnCreateSessionComplete(FName const SessionName, bool bSuccess) const
{
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not create session"))
		return;
	}

	auto Engine = GetEngine();
	if (!ensure(Engine)) return;

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green, TEXT("Hosting"));

	auto World = GetWorld();
	if (!ensure(World)) return;

	World->ServerTravel("/Game/BattleGrids/Levels/DefaultGameMap?listen");
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
	UE_LOG(LogTemp, Warning, TEXT("Finished Find Session"))
	if (bSuccess && SessionSearch.IsValid() && Menu)
	{
		TArray<FString> ServerNames;
		
		for (auto Result : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Result: %s"), *Result.GetSessionIdStr())
			ServerNames.Add(Result.GetSessionIdStr());
		}

		Menu->SetServerList(ServerNames);
	}
}
