// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/BGToken.h"

#include "Engine/DemoNetDriver.h"

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

void ABGToken::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGToken, PlayerPermissions)
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
		return true;
	return false;
}

void ABGToken::SetTokenPhysicsAndCollision_Implementation(bool const bPhysicsOn, bool const bGravityOn,
                                                             ECollisionEnabled::Type const CollisionType)
{
	StaticMeshComponent->SetSimulatePhysics(bPhysicsOn);
	StaticMeshComponent->SetEnableGravity(bGravityOn);
	StaticMeshComponent->SetCollisionEnabled(CollisionType);
}

bool ABGToken::PlayerHasPermissions(ABGPlayerState const* PlayerState)
{
	for (auto It : PlayerPermissions)
		if (It == PlayerState)
			return true;

	return false;
}

bool ABGToken::AddPlayerToPermissionsArray(ABGPlayerState* PlayerStateToAdd)
{
	if (PlayerPermissions.IsValidIndex(PlayerPermissions.AddUnique(PlayerStateToAdd)))
	{
		return true;
	}
	return false;
}

bool ABGToken::RemovePlayerFromPermissionsArray(ABGPlayerState* PlayerStateToRemove)
{
	if (PlayerPermissions.Contains(PlayerStateToRemove))
	{
		if (PlayerPermissions.Remove(PlayerStateToRemove) != 0)
		{
			return true;
		}
	}
	return false;
}

// Called when the game starts or when spawned
void ABGToken::BeginPlay()
{
	Super::BeginPlay();

	SetReplicatingMovement(true);
}

void ABGToken::FellOutOfWorld(const UDamageType& dmgType)
{
	Super::FellOutOfWorld(dmgType);

	Destroy();
}
