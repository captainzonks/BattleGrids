// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BGBoard.generated.h"

UCLASS()
class BATTLEGRIDS_API ABGBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABGBoard();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGBoard|Functions")
	void ShrinkBoard(int const& X, int const& Y);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGBoard|Functions")
	void GrowBoard(int const& X, int const& Y);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGBoard|Config")
	FVector2D BoardSize;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGBoard|Config")
	TArray<class ABGTile*> BoardTiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGBoard|Config")
	TSubclassOf<ABGTile> TileToSpawnReference;
};
