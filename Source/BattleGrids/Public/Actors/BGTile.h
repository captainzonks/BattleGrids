// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/BGActorInterface.h"
#include "Core/BGTypes.h"
#include "GameFramework/Actor.h"
#include "BGTile.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
class ABGBoard;

/**
* BattleGrids Tile class for tiles that make up the board
*/
UCLASS()
class BATTLEGRIDS_API ABGTile : public AActor, public IBGActorInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABGTile();

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "BGTile|Functions")
	void ToggleTileVisibility(bool bHide);

	//////////////////////
	/// Interface Implementation

	virtual void ToggleContextMenu() override;
	virtual void CloseContextMenu() override;
	virtual UBGContextMenu* GetContextMenu() override;

	///////////////////
	/// Getters

	FBGTileInfo GetTileInfo() const { return TileInfo; }

	class UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

	class ABGBoard* GetBoardReference() const { return BoardReference; }

	///////////////////
	/// Setters

	void SetTileInfo(FBGTileInfo const& NewTileInfo) { TileInfo = NewTileInfo; }

	void SetBoardReference(class ABGBoard* NewBoard) { BoardReference = NewBoard; }

	UFUNCTION(Server, Reliable)
	void SetWidgetComponentClass(TSubclassOf<UUserWidget> InClass);


	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateContextMenuWidget();

	///////////////////
	/// Replication Functions

	UFUNCTION()
    void OnRep_ContextMenuClass();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* ContextMenuWidgetComponent;

	UPROPERTY(ReplicatedUsing = OnRep_ContextMenuClass, EditAnywhere, BlueprintReadWrite,
		meta = (ExposeOnSpawn = "true"), Category = "BGTile|Config")
	TSubclassOf<UUserWidget> ContextMenuClass;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Config")
	ABGBoard* BoardReference;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"), Category = "Config")
	FBGTileInfo TileInfo;
};
