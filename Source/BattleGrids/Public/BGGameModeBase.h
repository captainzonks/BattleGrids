// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameModeBase|Config")
	TArray<TSubclassOf<class ABGToken>> CharacterTokens;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameModeBase|Config")
	TArray<APlayerController*> ConnectedPlayers;
};
