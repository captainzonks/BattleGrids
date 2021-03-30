// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/BGTile.h"


#include "Engine/DemoNetDriver.h"
#include "Engine/EngineTypes.h"

// Sets default values
ABGTile::ABGTile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	RootComponent = StaticMeshComponent;
	StaticMeshComponent->SetCollisionProfileName("Tile");
	StaticMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, 1.f));
	StaticMeshComponent->SetIsReplicated(true);
}

void ABGTile::ToggleTileVisibility_Implementation(bool const bHide)
{
	StaticMeshComponent->SetVisibility(!bHide, true);
	StaticMeshComponent->SetCollisionEnabled(bHide
		                                         ? ECollisionEnabled::Type::QueryOnly
		                                         : ECollisionEnabled::Type::QueryAndPhysics);
	StaticMeshComponent->SetCanEverAffectNavigation(bHide ? false : true);
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, bHide ? ECR_Ignore : ECR_Block);
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, bHide ? ECR_Ignore : ECR_Block);
}

// Called when the game starts or when spawned
void ABGTile::BeginPlay()
{
	Super::BeginPlay();
}

void ABGTile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGTile, BoardReference)
	DOREPLIFETIME(ABGTile, TileInfo)
}
