// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "UI/BGMenuInterface.h"

#include "CoreMinimal.h"

#include "BGTypes.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "BGGameInstance.generated.h"

class FOnlineSessionSearch;
class UBGSaveGame;
class UBGMainMenu;
class UBGLobbyMenu;
class UBGInGameMenu;
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

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Functions")
	void LoadMenuWidget();

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Functions")
	void LoadLobbyWidget();

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Functions")
	void InGameLoadMenuWidget();

	UFUNCTION(Exec, Category = "BGGameInstance|Functions")
	virtual void Host(FString const& ServerName) override;

	UFUNCTION(Exec, Category = "BGGameInstance|Functions")
	virtual void Join(uint32 const& Index, FBGServerData const& InServerData) override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Functions")
	virtual void LoadMainMenu() override;

	UFUNCTION(BlueprintCallable, Category = "BGGameInstance|Functions")
	virtual void RefreshServerList() override;

	void CreateSession() const;

	UBGLobbyMenu* GetLobby() const { return Lobby; }

	FBGServerData GetInitialServerData() const { return InitialServerData; }

protected:

	// Delegate Functions, called by the SessionInterface
	void OnCreateSessionComplete(FName const SessionName, bool bSuccess) const;
	void OnDestroySessionComplete(FName const SessionName, bool bSuccess);
	void OnFindSessionsComplete(bool bSuccess);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnNetworkFailure(UWorld* World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	UBGSaveGame* SaveGame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	TSubclassOf<UUserWidget> MainMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	TSubclassOf<UUserWidget> LobbyMenuClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameInstance|Config")
	TSubclassOf<UUserWidget> InGameMenuClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGGameInstance|Config")
	UBGMainMenu* Menu;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGGameInstance|Config")
	UBGLobbyMenu* Lobby;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BGGameInstance|Config")
	UBGInGameMenu* InGameMenu;

	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FBGServerData InitialServerData;
};
