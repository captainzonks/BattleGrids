// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BGTypes.h"
#include "GameFramework/Actor.h"
#include "BGTile.generated.h"

UCLASS()
class BATTLEGRIDS_API ABGTile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABGTile();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "BGTile|Functions")
	void ToggleTileVisibility(bool bHide);

	///////////////////
	/// Getters

	FBGTileInfo GetTileInfo() const { return TileInfo; }

	class ABGBoard* GetBoardReference() const { return BoardReference; }

	///////////////////
	/// Setters

	void SetTileInfo(FBGTileInfo const& NewTileInfo) { TileInfo = NewTileInfo; }

	void SetBoardReference(class ABGBoard* NewBoard) { BoardReference = NewBoard; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Config")
	class ABGBoard* BoardReference;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Config")
	FBGTileInfo TileInfo;
};
