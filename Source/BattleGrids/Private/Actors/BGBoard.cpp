// © 2021 Matthew Barham. All Rights Reserved.


#include "Actors/BGBoard.h"


#include "Actors/BGTile.h"
#include "Engine/DemoNetDriver.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABGBoard::ABGBoard()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ABGBoard::BuildBoard_Implementation(FVector const& CenteredLocation, int const& X, int const& Y)
{
	// Do not execute if BoardTiles contains Tiles already
	if (TileToSpawnReference.Get()->IsValidLowLevel() && BoardTiles.Num() == 0)
	{
		BoardSize.X = X;
		BoardSize.Y = Y;
		float const SectorSize = 100.f;

		// 2D Grid Execution Macro, rewritten in C++
		for (auto OuterIndex{0}; OuterIndex <= Y - 1; ++OuterIndex)
			for (auto InnerIndex{0}; InnerIndex <= X - 1; ++InnerIndex)
			{
				// Prepare a fresh Transform
				FVector SpawnLocation =
					FVector(static_cast<float>(InnerIndex) - static_cast<float>(X) / 2.f,
					         static_cast<float>(OuterIndex) - static_cast<float>(Y) / 2.f,
					         0.f)
						* SectorSize
					* 1.f
					+ CenteredLocation;

				FTransform SpawnTransform;
				SpawnTransform.SetLocation(SpawnLocation);

				// Prepare a Tile to spawn using the blueprint reference
				ABGTile* TileToSpawn = GetWorld()->SpawnActorDeferred<ABGTile>(TileToSpawnReference, SpawnTransform);
				TileToSpawn->SetBoardReference(this);
				TileToSpawn->SetTileInfo(FBGTileInfo(InnerIndex, OuterIndex, 0));

				// Spawn the new Tile and add it to the board's array
				if (auto SpawnedTile = Cast<ABGTile>(UGameplayStatics::FinishSpawningActor(TileToSpawn, SpawnTransform))
				)
				{
					BoardTiles.Add(SpawnedTile);
					SpawnedTile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
				}
			}
	}
}

void ABGBoard::ShrinkBoard_Implementation(int const& X, int const& Y)
{
	int TilesToRemoveCount{};

	for (auto Tile : BoardTiles)
		// Check if Tile is valid and not pending kill,
		// then check if its X or Y is greater than the incoming shrink size
		if (Tile->IsValidLowLevel() && !Tile->IsPendingKillOrUnreachable() && Tile->GetTileInfo().X > X - 1 || Tile->
			GetTileInfo().Y > Y - 1)
		{
			Tile->Destroy();
			++TilesToRemoveCount;
		}

	// After tiles have been marked to kill, clean out the BoardTiles array
	// with a reverse for loop to as to not encounter off-by-one errors
	for (int i{BoardTiles.Num() - 1}; TilesToRemoveCount != 0; --i)
		if (BoardTiles[i]->IsPendingKill())
		{
			BoardTiles.RemoveAt(i);
			--TilesToRemoveCount;
		}

	BoardSize.X = X;
	BoardSize.Y = Y;
}

void ABGBoard::GrowBoard_Implementation(int const& X, int const& Y)
{
	// Make sure the Blueprint Tile reference has been set in the Board defaults
	if (TileToSpawnReference.Get()->IsValidLowLevel())
	{
		TArray<ABGTile*> NewTileArray;

		// Grows board in the X-axis
		for (auto Tile : BoardTiles)
			// Check if Tile is valid and not pending kill, if it's the current final X-row,
			// and its X is less than the new final X-row
			if (Tile->IsValidLowLevel() && !Tile->IsPendingKillOrUnreachable() && Tile->GetTileInfo().X == BoardSize.X -
				1
				&& Tile->GetTileInfo().X < X)
			{
				// Prepare the new Tile's transform location to be 100cm spaced beyond the current Tile in the X-axis
				FTransform SpawnTransform = Tile->GetActorTransform();
				SpawnTransform.SetLocation(SpawnTransform.GetLocation() + FVector(100.f, 0, 0));
				// reset the scale to 1,1,1 to prevent quartering the copied 0.25f Z-axis off the reference Tile's transform
				SpawnTransform.SetScale3D(FVector::OneVector);

				// Prepare a Tile to spawn using the blueprint reference
				ABGTile* TileToSpawn = GetWorld()->SpawnActorDeferred<ABGTile>(TileToSpawnReference, SpawnTransform);

				// Set the deferred Tile's properties
				TileToSpawn->SetTileInfo(FBGTileInfo(Tile->GetTileInfo().X + 1, Tile->GetTileInfo().Y,
				                                     Tile->GetTileInfo().Z));
				TileToSpawn->SetBoardReference(this);

				// Spawn the new Tile and add it to the board's array
				if (auto SpawnedTile = Cast<ABGTile>(UGameplayStatics::FinishSpawningActor(TileToSpawn, SpawnTransform))
				)
				{
					NewTileArray.Add(SpawnedTile);
					SpawnedTile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
				}
			}

		BoardTiles.Append(NewTileArray);
		NewTileArray.Empty();

		BoardSize.X = X;

		// Grows board in the Y-axis
		for (auto Tile : BoardTiles)
			// Check if Tile is valid and not pending kill,
			// then check if it's the current final X-row, and its X is less than the new final X-row
			if (Tile->IsValidLowLevel() && !Tile->IsPendingKillOrUnreachable() && Tile->GetTileInfo().Y == BoardSize.Y -
				1
				&& Tile->GetTileInfo().Y < Y)
			{
				// Prepare the new Tile's transform location to be 100cm spaced beyond the current Tile in the Y-axis
				FTransform SpawnTransform = Tile->GetActorTransform();
				SpawnTransform.SetLocation(SpawnTransform.GetLocation() + FVector(0, 100.f, 0));
				// reset the scale to 1,1,1 to prevent quartering the copied 0.25f Z-axis off the reference Tile's transform
				SpawnTransform.SetScale3D(FVector::OneVector);

				// Prepare a Tile to spawn using the blueprint reference
				ABGTile* TileToSpawn = GetWorld()->SpawnActorDeferred<ABGTile>(TileToSpawnReference, SpawnTransform);

				// Set the deferred Tile's properties
				TileToSpawn->SetTileInfo(FBGTileInfo(Tile->GetTileInfo().X, Tile->GetTileInfo().Y + 1,
				                                     Tile->GetTileInfo().Z));
				TileToSpawn->SetBoardReference(this);

				// Spawn the new Tile and add it to the board's array
				if (auto SpawnedTile = Cast<ABGTile>(UGameplayStatics::FinishSpawningActor(TileToSpawn, SpawnTransform))
				)
				{
					NewTileArray.Add(SpawnedTile);
					SpawnedTile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
				}
			}

		BoardTiles.Append(NewTileArray);

		BoardSize.Y = Y;
	}
}

// Called when the game starts or when spawned
void ABGBoard::BeginPlay()
{
	Super::BeginPlay();

	SetReplicatingMovement(true);
}

void ABGBoard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGBoard, BoardTiles)
	DOREPLIFETIME(ABGBoard, BoardSize)
	DOREPLIFETIME(ABGBoard, TileToSpawnReference)
}
