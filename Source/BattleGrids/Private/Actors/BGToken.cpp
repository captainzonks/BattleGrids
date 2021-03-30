// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/BGToken.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DemoNetDriver.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ABGToken::ABGToken()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// AI Controller is NECESSARY in order for AddMovementInput() to work!!!
	AutoPossessAI = EAutoPossessAI::Spawned;

	auto Capsule = GetCapsuleComponent();

	// Capsule->SetIsReplicated(true);
	Capsule->bDynamicObstacle = true;
	Capsule->InitCapsuleSize(20.f, 60.f);
	Capsule->SetCollisionProfileName("TokenCapsule");

	TokenBaseStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Token Base Mesh"));
	TokenBaseStaticMeshComponent->SetupAttachment(GetCapsuleComponent());
	TokenBaseStaticMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -60.f));
	TokenBaseStaticMeshComponent->SetCollisionProfileName("Token");
	TokenBaseStaticMeshComponent->SetIsReplicated(true);

	TokenModelStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Token Model Mesh"));
	TokenModelStaticMeshComponent->SetCollisionProfileName("Token");
	TokenModelStaticMeshComponent->SetIsReplicated(true);
}

void ABGToken::InitializeMeshAndMaterial_Implementation(UStaticMesh* StaticMesh,
                                                        UMaterialInstance* MaterialInstance,
                                                        UStaticMesh* BaseStaticMesh) const
{
	if (BaseStaticMesh && StaticMesh && MaterialInstance)
	{
		TokenBaseStaticMeshComponent->SetStaticMesh(BaseStaticMesh);
		TokenBaseStaticMeshComponent->SetMaterial(0, MaterialInstance);
		TokenModelStaticMeshComponent->SetStaticMesh(StaticMesh);
		TokenModelStaticMeshComponent->SetMaterial(0, MaterialInstance);
	}
}

void ABGToken::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGToken, PlayerPermissions)
}

void ABGToken::ToggleLockTokenInPlace_Implementation(bool bLock)
{
	TokenBaseStaticMeshComponent->GetBodyInstance()->bLockXTranslation = bLock;
	TokenBaseStaticMeshComponent->GetBodyInstance()->bLockYTranslation = bLock;
	TokenBaseStaticMeshComponent->GetBodyInstance()->bLockZTranslation = bLock;
	TokenBaseStaticMeshComponent->GetBodyInstance()->bLockXRotation = bLock;
	TokenBaseStaticMeshComponent->GetBodyInstance()->bLockYRotation = bLock;
	TokenBaseStaticMeshComponent->GetBodyInstance()->bLockZRotation = bLock;
	TokenBaseStaticMeshComponent->GetBodyInstance()->SetDOFLock(EDOFMode::SixDOF);
	TokenModelStaticMeshComponent->GetBodyInstance()->bLockXTranslation = bLock;
	TokenModelStaticMeshComponent->GetBodyInstance()->bLockYTranslation = bLock;
	TokenModelStaticMeshComponent->GetBodyInstance()->bLockZTranslation = bLock;
	TokenModelStaticMeshComponent->GetBodyInstance()->bLockXRotation = bLock;
	TokenModelStaticMeshComponent->GetBodyInstance()->bLockYRotation = bLock;
	TokenModelStaticMeshComponent->GetBodyInstance()->bLockZRotation = bLock;
	TokenModelStaticMeshComponent->GetBodyInstance()->SetDOFLock(EDOFMode::SixDOF);
}

bool ABGToken::GetIsTokenLocked() const
{
	auto const Body = TokenModelStaticMeshComponent->GetBodyInstance();
	if (Body->bLockXTranslation && Body->bLockYTranslation && Body->bLockZTranslation)
		return true;
	return false;
}

bool ABGToken::PlayerHasPermissions(ABGPlayerState const* PlayerStateToCheck)
{
	for (auto It : PlayerPermissions)
		if (It == PlayerStateToCheck)
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

	CharacterAIController = Cast<AAIController>(GetController());
	if (CharacterAIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("PawnAIController Successfully Initialized"))
	}

	SetReplicatingMovement(true);
	TokenModelStaticMeshComponent->SetSimulatePhysics(false);
	TokenModelStaticMeshComponent->AttachToComponent(TokenBaseStaticMeshComponent,
	                                                 FAttachmentTransformRules(EAttachmentRule::KeepRelative, true),
	                                                 TEXT("ModelRoot"));
}

void ABGToken::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABGToken::FellOutOfWorld(const UDamageType& dmgType)
{
	Super::FellOutOfWorld(dmgType);

	Destroy();
}
