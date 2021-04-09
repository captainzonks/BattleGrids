// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BGPlayerController.generated.h"

struct FBGPlayerInfo;
class UBGGameInstance;

/**
 * Parent Player Controller class for BattleGrids
 */
UCLASS()
class BATTLEGRIDS_API ABGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "BGPlayerController|Functions")
	void ToggleLoadingState(bool const bIsLoading);

	UFUNCTION(Server, Reliable, Category = "BGPlayerController|Functions|Network")
	void ServerToggleLoadingState(bool const bIsLoading);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "BGLobbyPlayerController|UI")
	void UpdateUI(TArray<FBGPlayerInfo> const& InPlayerInfoArray);

protected:
	ABGPlayerController();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	uint8 bLoading : 1;
};
