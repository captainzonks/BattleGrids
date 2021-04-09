// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Core/BGTypes.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BGGameModeBase.generated.h"


/**
 * Primary Game Mode Base class for gameplay
 */
UCLASS()
class BATTLEGRIDS_API ABGGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:

	///////////////////////
	/// Server Variables

	UFUNCTION(BlueprintCallable, Category = "ABGGameModeBase|Functions")
	void UpdatePlayerInfoInArray(FBGPlayerInfo const& InPlayerInfo);

	UFUNCTION(BlueprintCallable, Category = "ABGGameModeBase|Functions")
	void RemovePlayerInfoFromArray(FBGPlayerInfo const& InPlayerInfo);

protected:
	
	virtual void BeginPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	void AddPlayerInfoToArray(FBGPlayerInfo const& InPlayerInfo);

	void SetUpdateTimer();

	UFUNCTION()
    void UpdateConnectedPlayersUI();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameModeBase|Config")
	TArray<FBGPlayerInfo> ConnectedPlayersInfo;

	FTimerHandle UpdateTimer;
};
