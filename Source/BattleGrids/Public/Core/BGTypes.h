// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"
#include "BGTypes.generated.h"

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
	UStaticMesh* PrimaryStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* PrimaryMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* SecondaryStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* SecondaryMaterialInstance;

	FBGStructureInfo() = default;

	FBGStructureInfo(UStaticMesh* NewPrimaryStaticMesh, UMaterialInstance* NewPrimaryMaterialInstance,
	                 FTransform const NewTransform)
		: PrimaryStaticMesh(NewPrimaryStaticMesh), PrimaryMaterialInstance(NewPrimaryMaterialInstance),
		  Transform(NewTransform)
	{
		SecondaryStaticMesh = nullptr;
		SecondaryMaterialInstance = nullptr;
	}

	FBGStructureInfo(UStaticMesh* NewPrimaryStaticMesh, UMaterialInstance* NewPrimaryMaterialInstance,
	                 FTransform const NewTransform,
	                 UStaticMesh* NewSecondaryStaticMesh, UMaterialInstance* NewSecondaryMaterialInstance)
		: PrimaryStaticMesh(NewPrimaryStaticMesh), PrimaryMaterialInstance(NewPrimaryMaterialInstance),
		  Transform(NewTransform),
		  SecondaryStaticMesh(NewSecondaryStaticMesh), SecondaryMaterialInstance(NewSecondaryMaterialInstance)
	{
	}
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

	FBGTileInfo() = default;

	FBGTileInfo(int const NewX, int const NewY, int const NewZ)
		: X(NewX), Y(NewY), Z(NewZ)
	{
	}
};
