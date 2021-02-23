// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"
#include "BGTypes.generated.h"

USTRUCT(BlueprintType)
struct  FBGTokenBank : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TokenName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ABGToken> TokenClassReference;
};

USTRUCT(BlueprintType)
struct FBGPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bGameMasterPermissions : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PlayerColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bReady : 1;
};

USTRUCT(BlueprintType)
struct FBGTileInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Y;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Z;
};