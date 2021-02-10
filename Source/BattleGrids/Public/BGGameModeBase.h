// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/GameModeBase.h"
#include "BGGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEGRIDS_API ABGGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGameModeBase|Config")
	TArray<TSubclassOf<class ABGToken>> CharacterTokens;
};
