// Fill out your copyright notice in the Description page of Project Settings.


#include "BGTile.h"

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
}

// Called when the game starts or when spawned
void ABGTile::BeginPlay()
{
	Super::BeginPlay();
}
