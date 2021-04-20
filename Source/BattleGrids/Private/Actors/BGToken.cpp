// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/BGToken.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DemoNetDriver.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/BGContextMenu.h"

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
	TokenBaseStaticMeshComponent->SetupAttachment(Capsule);
	TokenBaseStaticMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -60.f));
	TokenBaseStaticMeshComponent->SetCollisionProfileName("Token");
	TokenBaseStaticMeshComponent->SetIsReplicated(true);

	TokenModelStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Token Model Mesh"));
	TokenModelStaticMeshComponent->SetCollisionProfileName("Token");
	TokenModelStaticMeshComponent->SetIsReplicated(true);

	ContextMenuWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Context Menu Widget"));
	ContextMenuWidgetComponent->SetupAttachment(Capsule);
	ContextMenuWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	ContextMenuWidgetComponent->SetPivot(FVector2D(0.f, 0.f));
	ContextMenuWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	ContextMenuWidgetComponent->SetDrawAtDesiredSize(true);
	ContextMenuWidgetComponent->SetVisibility(false);
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

	Cast<UBGContextMenu>(ContextMenuWidgetComponent->GetWidget())->SetParent(this);
}

void ABGToken::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ABGToken::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGToken, PlayerPermissions)
}

void ABGToken::SetWidgetComponentClass(TSubclassOf<UUserWidget> const& InClass) const
{
	ContextMenuWidgetComponent->SetWidgetClass(InClass);
}

void ABGToken::ToggleContextMenu()
{
	if (ContextMenuWidgetComponent && ContextMenuWidgetComponent->GetWidget())
	{
		ContextMenuWidgetComponent->SetWorldLocation(GetContextMenu()->GetHitResult().ImpactPoint);
		ContextMenuWidgetComponent->ToggleVisibility();
	}
}

void ABGToken::CloseContextMenu()
{
	if (ContextMenuWidgetComponent)
	{
		ContextMenuWidgetComponent->SetVisibility(false);
	}
}

UBGContextMenu* ABGToken::GetContextMenu()
{
	return Cast<UBGContextMenu>(ContextMenuWidgetComponent->GetWidget());
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


void ABGToken::ToggleLockTokenInPlace_Implementation(bool const bLock)
{
	bLock ? GetCharacterMovement()->DisableMovement() : GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

bool ABGToken::GetIsTokenLocked() const
{
	if (GetCharacterMovement()->MovementMode == MOVE_None)
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

void ABGToken::FellOutOfWorld(const UDamageType& dmgType)
{
	Super::FellOutOfWorld(dmgType);

	Destroy();
}
