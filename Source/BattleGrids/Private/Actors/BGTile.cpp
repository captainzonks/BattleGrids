// © 2021 Matthew Barham. All Rights Reserved.

#include "Actors/BGTile.h"

#include "Components/WidgetComponent.h"
#include "Engine/DemoNetDriver.h"
#include "Engine/EngineTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/BGContextMenu.h"

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

	ContextMenuWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Context Menu Widget"));
	ContextMenuWidgetComponent->SetupAttachment(RootComponent);
	ContextMenuWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	ContextMenuWidgetComponent->SetPivot(FVector2D(0.f, 0.f));
	ContextMenuWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	ContextMenuWidgetComponent->SetDrawAtDesiredSize(true);
	ContextMenuWidgetComponent->SetVisibility(false);
}

// Called when the game starts or when spawned
void ABGTile::BeginPlay()
{
	Super::BeginPlay();

	// run this on the Server immediately, but wait for OnRep_ContextMenuClass() for Clients
	if (UKismetSystemLibrary::IsServer(this))
	{
		UpdateContextMenuWidget();
	}
}

void ABGTile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGTile, BoardReference)
	DOREPLIFETIME(ABGTile, TileInfo)
	DOREPLIFETIME(ABGTile, ContextMenuClass)
}

void ABGTile::OnRep_ContextMenuClass()
{
	UpdateContextMenuWidget();
}

void ABGTile::UpdateContextMenuWidget()
{
	// if Client, set the Widget Class
	if (!UKismetSystemLibrary::IsServer(this) && ContextMenuClass.GetDefaultObject())
	{
		ContextMenuWidgetComponent->SetWidgetClass(ContextMenuClass);
	}

	// Set Parent variable on the ContextMenu
	auto ContextMenu = GetContextMenu();
	if (ContextMenu)
	{
		ContextMenu->SetParent(this);
	}
}

void ABGTile::SetWidgetComponentClass_Implementation(TSubclassOf<UUserWidget> InClass)
{
	ContextMenuClass.operator=(InClass);
	ContextMenuWidgetComponent->SetWidgetClass(InClass);
	UpdateContextMenuWidget();
}

void ABGTile::ToggleContextMenu()
{
	if (ContextMenuWidgetComponent && ContextMenuWidgetComponent->GetWidget())
	{
		ContextMenuWidgetComponent->SetWorldLocation(GetContextMenu()->GetHitResult().ImpactPoint);
		ContextMenuWidgetComponent->ToggleVisibility();
	}
}

void ABGTile::CloseContextMenu()
{
	if (ContextMenuWidgetComponent)
	{
		ContextMenuWidgetComponent->SetVisibility(false);
	}
}

UBGContextMenu* ABGTile::GetContextMenu()
{
	return Cast<UBGContextMenu>(ContextMenuWidgetComponent->GetWidget());
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
