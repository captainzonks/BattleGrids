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
	StaticMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, 0.25f));
	StaticMeshComponent->SetIsReplicated(true);
}

void ABGTile::ToggleTileVisibility_Implementation(bool const bHide)
{
	StaticMeshComponent->SetVisibility(!bHide, true);
	StaticMeshComponent->SetCollisionEnabled(bHide
		                                         ? ECollisionEnabled::Type::QueryOnly
		                                         : ECollisionEnabled::Type::QueryAndPhysics);
	StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
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
