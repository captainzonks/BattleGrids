// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "Components/SplineComponent.h"
#include "Engine/DataTable.h"
#include "BGTypes.generated.h"

/** Custom Collision macros */
#define ECC_TOKEN ECC_GameTraceChannel1
#define ECC_TILE ECC_GameTraceChannel2
#define ECC_STRUCTURE ECC_GameTraceChannel3
#define ECC_TOKEN_CAPSULE ECC_GameTraceChannel4
#define ECC_BASE ECC_GameTraceChannel5
#define ECC_RIGHT_CLICK ECC_GameTraceChannel6
#define ECC_GRAB ECC_GameTraceChannel7
#define ECC_STRUCTURES ECC_GameTraceChannel8
#define ECC_TILES ECC_GameTraceChannel9
#define ECC_TOKENS ECC_GameTraceChannel10
#define ECC_BASES ECC_GameTraceChannel11

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
enum class EBGClassCategory : uint8
{
	None UMETA(DisplayName = "None"),
	Character UMETA(DisplayName = "Miniatures"),
	Actor UMETA(DisplayName = "Environmental")
};

UENUM(BlueprintType)
enum class EBGActorType : uint8
{
	None UMETA(DisplayName = "None"),
	Structure UMETA(DisplayName = "Wall"),
	Board UMETA(DisplayName = "Board"),
	Tile UMETA(DisplayName = "Tile"),
	Light UMETA(DisplayName = "Light"),
	Door UMETA(DisplayName = "Door")
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
	EBGClassCategory ClassCategory;
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

	FBGPlayerInfo(): bGameMasterPermissions(0), bReady(0)
	{
	}

	explicit FBGPlayerInfo(FText const& InName, int const& InPlayerID)
		: PlayerName(InName), PlayerID(InPlayerID), bGameMasterPermissions(0), bReady(0)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PlayerName{FText::FromString("DefaultPlayerName")};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PlayerID{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PlayerColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bGameMasterPermissions : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bReady : 1;

	bool operator==(FBGPlayerInfo const& Right) const
	{
		return PlayerID == Right.PlayerID;
	}
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

USTRUCT(BlueprintType)
struct FBGCharacterModelSaveInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CharacterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CharacterLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CharacterModelStaticMeshName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CharacterModelMaterialName;

	FBGCharacterModelSaveInfo() = default;

	explicit FBGCharacterModelSaveInfo(int const InCharacterID, FName const InCharacterName,
	                                   FVector const InCharacterLocation,
	                                   FName const InCharacterModelStaticMeshName,
	                                   FName const InCharacterModelMaterialName)
		: CharacterID(InCharacterID), CharacterName(InCharacterName), CharacterLocation(InCharacterLocation),
		  CharacterModelStaticMeshName(InCharacterModelStaticMeshName),
		  CharacterModelMaterialName(InCharacterModelMaterialName)
	{
	}

	bool operator==(FBGCharacterModelSaveInfo const& Right) const
	{
		return this->CharacterID == Right.CharacterID;
	}
};

USTRUCT(BlueprintType)
struct FBGWallSplineSaveInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int WallSplineID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY()
	FSplineCurves SplineCurves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WallStaticMeshName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WallMaskedMaterialInstanceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CornerStaticMeshName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CornerMaskedMaterialInstanceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BaseStaticMeshName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BaseMaterialInstanceName;

	FBGWallSplineSaveInfo() = default;

	explicit FBGWallSplineSaveInfo(int const InWallSplineID, FName const InTags, FVector const InLocation,
	                               FName const InWallStaticMeshName,
	                               FName const InWallMaskedMaterialInstanceName, FName const InCornerStaticMeshName,
	                               FName const InCornerMaskedMaterialInstanceName,
	                               FName const InBaseStaticMeshName, FName const InBaseMaterialInstanceName)
		: WallSplineID(InWallSplineID), Tags(InTags), Location(InLocation),
		  WallStaticMeshName(InWallStaticMeshName),
		  WallMaskedMaterialInstanceName(InWallMaskedMaterialInstanceName),
		  CornerStaticMeshName(InCornerStaticMeshName),
		  CornerMaskedMaterialInstanceName(InCornerMaskedMaterialInstanceName),
		  BaseStaticMeshName(InBaseStaticMeshName), BaseMaterialInstanceName(InBaseMaterialInstanceName)
	{
	}

	bool operator==(FBGWallSplineSaveInfo const& Right) const
	{
		return this->WallSplineID == Right.WallSplineID;
	}
};
