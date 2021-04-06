// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"
#include "BGTypes.generated.h"

USTRUCT(BlueprintType)
struct FBGServerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CurrentPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int MaxPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HostUsername;
};

UENUM(BlueprintType)
enum class EBGObjectType : uint8
{
	None UMETA(DisplayName = "None"),
	Token UMETA(DisplayName = "Token"),
	Structure UMETA(DisplayName = "Structure"),
	Board UMETA(DisplayName = "Board")
};

UENUM(BlueprintType)
enum class EBGControlMode : uint8
{
	Build UMETA(DisplayName = "Build"),
	Edit UMETA(DisplayName = "Edit"),
	Move UMETA(DisplayName = "Move")
};

USTRUCT(BlueprintType)
struct FBGStaticMeshBank : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText StaticMeshName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBGObjectType ObjectType;
};

USTRUCT(BlueprintType)
struct FBGMaterialInstanceBank : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MaterialInstanceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UMaterialInstance* MaterialInstance;
};

USTRUCT(BlueprintType)
struct FBGStructureInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* WallStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* WallMaskedMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* CornerStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* CornerMaskedMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* BaseStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* BaseMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;

	FBGStructureInfo() = default;

	FBGStructureInfo(UStaticMesh* NewWallStaticMesh, UMaterialInstance* NewWallMaskedMaterialInstance,
	                 UStaticMesh* NewCornerStaticMesh, UMaterialInstance* NewCornerMaskedMaterialInstance,
	                 UStaticMesh* NewBaseStaticMesh, UMaterialInstance* NewBaseMaterialInstance,
	                 FTransform const NewTransform)
		: WallStaticMesh(NewWallStaticMesh), WallMaskedMaterialInstance(NewWallMaskedMaterialInstance),
		  CornerStaticMesh(NewCornerStaticMesh), CornerMaskedMaterialInstance(NewCornerMaskedMaterialInstance),
		  BaseStaticMesh(NewBaseStaticMesh), BaseMaterialInstance(NewBaseMaterialInstance), Transform(NewTransform)
	{
	}
};

USTRUCT(BlueprintType)
struct FBGPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlayerName {FText::FromString("DefaultPlayerName")};

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

	FBGTileInfo() = default;

	FBGTileInfo(int const NewX, int const NewY, int const NewZ)
		: X(NewX), Y(NewY), Z(NewZ)
	{
	}
};
