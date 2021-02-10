// Fill out your copyright notice in the Description page of Project Settings.


#include "BGTile.h"

// Sets default values
ABGTile::ABGTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	RootComponent = StaticMeshComponent;
	StaticMeshComponent->SetRelativeScale3D(FVector(1.f, 1.f, 0.25f));

}

// Called when the game starts or when spawned
void ABGTile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABGTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

