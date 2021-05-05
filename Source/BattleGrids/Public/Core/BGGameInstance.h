// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "BGTypes.h"
#include "UI/BGMenuInterface.h"

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "BGGameInstance.generated.h"

class UBGPlayerSave;
class UBGThinkingPopup;
class FOnlineSessionSearch;
class UBGGameHUD;
class UBGMainMenu;
class UBGLobbyMenu;
class UBGInGamePlayerList;
class UBGInGameMenu;
class UBGLoadingScreen;
class UUserWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSaveGame, FBGPlayerInfo const&, NewPlayerInfo);

/**
 * BattleGrids GameInstance class
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
	virtual void RefreshLoadGameList() override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Menu Interface")
	virtual void RefreshPlayerLists(TArray<FBGPlayerInfo> const& InPlayerInfo) override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Menu Interface")
	virtual void ToggleLoadingScreen(bool const bIsLoading) override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Menu Interface")
	virtual void ToggleThinkingPopup(bool const bThinking) override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Menu Interface")
	virtual void SavePlayerInfo(FBGPlayerInfo const& InPlayerInfo) override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Menu Interface")
	virtual void SaveGameInfo() override;

	UFUNCTION(BlueprintCallable, Exec, Category = "BGGameInstance|Menu Interface")
	virtual void LoadGameInfo(int const& Index, UBGGameSave* InGameSave) override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Menu Interface")
	virtual void UpdatePlayerInfo(int const& Index, FBGPlayerInfo const& InPlayerInfo) override;

	///////////////////////
	/// Functions

	void CreateSession() const;

	bool GetLoading() const { return bLoading; }

	// UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Gameplay")
	// void SavePlayerInfo(FBGPlayerInfo const& InPlayerInfo);
	
	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Gameplay")
	bool LoadPlayerInfo(FBGPlayerInfo& OutPlayerInfo);

	//////////////////////
	/// UI Getters

	UBGLobbyMenu* GetLobby() const { return Lobby; }
	UBGGameHUD* GetGameHUD() const { return GameHUD; }
	UBGInGamePlayerList* GetInGamePlayerList() const { return InGamePlayerList; }

	/////////////////////
	/// Persistent Data

	FBGServerData GetServerData() const { return ServerData; }
	void SetServerData(FBGServerData const& InServerData) { ServerData = InServerData; }

	UBGPlayerSave* GetPlayerSave() const { return PlayerSave; }
	UBGGameSave* GetGameSave() const { return GameSave; }

	bool DoesSaveGameExist() const
	{
		if (PlayerSave) return true;
		return false;
	}

	/////////////////
	/// Delegates

	UPROPERTY(BlueprintAssignable, Category = "BGGameInstance|Delegates")
	FSaveGame OnSaveGame;


protected:

	void ShowLoadingScreen();
	void HideLoadingScreen();
	void ShowThinkingPopup();
	void HideThinkingPopup();

	//////////////////////
	/// Delegate Functions, called by the SessionInterface

	void OnCreateSessionComplete(FName const SessionName, bool bSuccess);
	void OnDestroySessionComplete(FName const SessionName, bool bSuccess);
	void OnFindSessionsComplete(bool bSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
	                      const FString& ErrorString);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	UBGPlayerSave* PlayerSave;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	UBGGameSave* GameSave;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "BGGameInstance|Config")
	TSubclassOf<UUserWidget> ThinkingPopupClass;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGGameInstance|Config")
	UBGThinkingPopup* ThinkingPopup;

	//////////////////
	/// Variables

	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	uint8 bLoading : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	FBGServerData ServerData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	FString DefaultPlayerSaveSlotName{"PlayerSave"};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	FString DefaultGameSaveSlotName{"GameSave"};
};