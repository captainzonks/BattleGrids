// Fill out your copyright notice in the Description page of Project Settings.


#include "BGToken.h"

// Sets default values
ABGToken::ABGToken()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	RootComponent = StaticMeshComponent;
	StaticMeshComponent->bIgnoreRadialForce = true;
	StaticMeshComponent->bIgnoreRadialImpulse = true;
	StaticMeshComponent->SetLinearDamping(2.f);
	StaticMeshComponent->SetAngularDamping(2.f);
}

void ABGToken::ToggleLockTokenInPlace_Implementation(bool bLock)
{
	StaticMeshComponent->GetBodyInstance()->bLockXTranslation = bLock;
	StaticMeshComponent->GetBodyInstance()->bLockYTranslation = bLock;
	StaticMeshComponent->GetBodyInstance()->bLockZTranslation = bLock;
	StaticMeshComponent->GetBodyInstance()->bLockXRotation = bLock;
	StaticMeshComponent->GetBodyInstance()->bLockYRotation = bLock;
	StaticMeshComponent->GetBodyInstance()->bLockZRotation = bLock;
	StaticMeshComponent->GetBodyInstance()->SetDOFLock(EDOFMode::SixDOF);
}

bool ABGToken::GetIsTokenLocked() const
{
	auto const Body = StaticMeshComponent->GetBodyInstance();
	if (Body->bLockXTranslation && Body->bLockYTranslation && Body->bLockZTranslation)
	{
		return true;
	}
	return false;
}

void ABGToken::ToggleTokenPhysicsAndCollision_Implementation(bool const bPhysicsOn, bool const bGravityOn,
                                                             ECollisionEnabled::Type const CollisionType)
{
	StaticMeshComponent->SetSimulatePhysics(bPhysicsOn);
	StaticMeshComponent->SetEnableGravity(bGravityOn);
	StaticMeshComponent->SetCollisionEnabled(CollisionType);
}

// Called when the game starts or when spawned
void ABGToken::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(true);
}
