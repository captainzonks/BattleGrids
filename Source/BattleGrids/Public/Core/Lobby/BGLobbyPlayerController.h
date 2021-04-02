// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BGPlayerController.h"
#include "Core/BGTypes.h"


#include "BGLobbyPlayerController.generated.h"

class UBGLobbyMenu;

/**
 * 
 */
UCLASS()
class BATTLEGRIDS_API ABGLobbyPlayerController : public ABGPlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void UpdateServerData(FBGServerData const& InServerData);
	
	UFUNCTION(Client, Reliable)
	void UpdateLobbyInformation();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	FBGServerData ServerData;
};
