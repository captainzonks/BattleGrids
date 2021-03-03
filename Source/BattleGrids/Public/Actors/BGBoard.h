// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BGBoard.generated.h"

class ABGTile;

UCLASS()
class BATTLEGRIDS_API ABGBoard : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABGBoard();

	/* Build out a new grid of tiles.
	 * Will not execute if BoardTiles array is not empty. */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGBoard|Functions")
	void BuildBoard(FVector const& CenteredLocation, int const& X, int const& Y);

	/* Shrinks board in both X and Y axes to the specified new X,Y size */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGBoard|Functions")
	void ShrinkBoard(int const& X, int const& Y);

	/* Grows board in both X and Y axes to the specified new X,Y size */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGBoard|Functions")
	void GrowBoard(int const& X, int const& Y);

	///////////////////
	/// Getters

	FVector2D GetBoardSize() const { return BoardSize; }

	int GetZedLevel() const { return ZedLevel; }

	TArray<ABGTile*> GetBoardTiles() const { return BoardTiles; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGBoard|Config")
	FVector2D BoardSize;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGBoard|Config")
	int ZedLevel{};

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGBoard|Config")
	TArray<ABGTile*> BoardTiles;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGBoard|Config")
	TSubclassOf<ABGTile> TileToSpawnReference;
};
