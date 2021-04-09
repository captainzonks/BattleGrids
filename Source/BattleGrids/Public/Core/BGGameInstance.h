// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "BGTypes.h"
#include "UI/BGMenuInterface.h"

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "BGGameInstance.generated.h"

class FOnlineSessionSearch;
class UBGGameHUD;
class UBGSaveGame;
class UBGMainMenu;
class UBGLobbyMenu;
class UBGInGamePlayerList;
class UBGInGameMenu;
class UBGLoadingScreen;
class UUserWidget;

/**
 * 
 */
UCLASS()
class BATTLEGRIDS_API UBGGameInstance : public UGameInstance, public IBGMenuInterface
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	/////////////////////
	/// UI Creation

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|UI")
	void LoadMainMenuWidget();

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|UI")
	void LoadLobbyWidget();

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|UI")
	void LoadGameHUDWidget();

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|UI")
	void LoadInGamePlayerListWidget();

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|UI")
	void InGameLoadMenuWidget();

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|UI")
	void ShowLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|UI")
	void HideLoadingScreen();

	/////////////////////
	/// Menu Interface Implementations

	UFUNCTION(BlueprintCallable, Exec, Category = "BGGameInstance|Menu Interface")
	virtual void Host(FString const& ServerName) override;

	UFUNCTION(BlueprintCallable, Exec, Category = "BGGameInstance|Menu Interface")
	virtual void Join(int const& Index, FBGServerData const& InServerData) override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Menu Interface")
	virtual void StartGame() override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Menu Interface")
	virtual void LoadMainMenu() override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Menu Interface")
	virtual void RefreshServerList() override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Menu Interface")
	virtual void RefreshPlayerLists(TArray<FBGPlayerInfo> const& InPlayerInfo) override;

	///////////////////////
	/// Functions

	void CreateSession() const;

	//////////////////////
	/// UI Getters

	UBGLobbyMenu* GetLobby() const { return Lobby; }
	UBGGameHUD* GetGameHUD() const { return GameHUD; }
	UBGInGamePlayerList* GetInGamePlayerList() const { return InGamePlayerList; }

	/////////////////////
	/// Persistent Data

	FBGServerData GetServerData() const { return ServerData; }
	void SetServerData(FBGServerData const& InServerData) { ServerData = InServerData; }

protected:

	//////////////////////
	/// Delegate Functions, called by the SessionInterface

	void OnCreateSessionComplete(FName const SessionName, bool bSuccess);
	void OnDestroySessionComplete(FName const SessionName, bool bSuccess);
	void OnFindSessionsComplete(bool bSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
	                      const FString& ErrorString);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	UBGSaveGame* SaveGame;

	///////////////////////
	/// Blueprint References, to be set in Editor on defaults

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BGGameInstance|Config")
	TSubclassOf<UUserWidget> MainMenuClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BGGameInstance|Config")
	TSubclassOf<UUserWidget> LobbyMenuClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BGGameInstance|Config")
	TSubclassOf<UUserWidget> GameHUDClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BGGameInstance|Config")
	TSubclassOf<UUserWidget> InGamePlayerListClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BGGameInstance|Config")
	TSubclassOf<UUserWidget> InGameMenuClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BGGameInstance|Config")
	TSubclassOf<UUserWidget> LoadingScreenClass;

	///////////////////
	/// UI Pointers

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGGameInstance|Config")
	UBGMainMenu* Menu;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGGameInstance|Config")
	UBGLobbyMenu* Lobby;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGGameInstance|Config")
	UBGGameHUD* GameHUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGGameInstance|Config")
	UBGInGamePlayerList* InGamePlayerList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGGameInstance|Config")
	UBGInGameMenu* InGameMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGGameInstance|Config")
	UBGLoadingScreen* LoadingScreen;

	//////////////////
	/// Variables

	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	FBGServerData ServerData;
};
