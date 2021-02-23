// © 2021 Matthew Barham. All Rights Reserved.


#include "BGBoard.h"


#include "BGTile.h"
#include "Engine/DemoNetDriver.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABGBoard::ABGBoard()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ABGBoard::ShrinkBoard_Implementation(int const& X, int const& Y)
{
	for (auto Tile : BoardTiles)
	{
		if (Tile->GetTileInfo().X > X - 1 || Tile->GetTileInfo().Y > Y - 1)
		{
			Tile->Destroy();
		}
	}

	BoardSize.X = X;
	BoardSize.Y = Y;
}

void ABGBoard::GrowBoard_Implementation(int const& X, int const& Y)
{
	for (auto Tile : BoardTiles)
	{
		if (Tile->GetTileInfo().X == BoardSize.X - 1 && Tile->GetTileInfo().X < X)
		{
			FTransform SpawnTransform = Tile->GetActorTransform();
			SpawnTransform.SetLocation(SpawnTransform.GetLocation() + FVector(105.f, 0, 0));
			UGameplayStatics::BeginDeferredActorSpawnFromClass(this, TileToSpawnReference, SpawnTransform);
		}
	}
}

// Called when the game starts or when spawned
void ABGBoard::BeginPlay()
{
	Super::BeginPlay();
}

void ABGBoard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGBoard, BoardTiles)
	DOREPLIFETIME(ABGBoard, BoardSize)
}
