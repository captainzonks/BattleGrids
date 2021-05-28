// © 2021 Matthew Barham. All Rights Reserved.


#include "Actors/BGPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Core/Gameplay/BGGamePlayerController.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialParameterCollection.h"

// Sets default values
ABGPawn::ABGPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Component"));
	RootComponent = CapsuleComponent;
	CapsuleComponent->SetCapsuleHalfHeight(90.f);
	CapsuleComponent->SetCapsuleRadius(90.f);
	CapsuleComponent->SetSimulatePhysics(true);

	PawnMovementComponent = CreateDefaultSubobject<UPawnMovementComponent>(TEXT("Pawn Movement"));
	// PawnMovementComponent->SetIsReplicated(true);

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(
		TEXT("Floating Pawn Movement"));
	FloatingPawnMovement->SetIsReplicated(true);

	MainStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Main Body Mesh"));
	MainStaticMeshComponent->SetupAttachment(CapsuleComponent);
	MainStaticMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -50.f));

	PupilMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pupil Mesh"));
	PupilMeshComponent->SetupAttachment(MainStaticMeshComponent);
	PupilMeshComponent->SetRelativeLocation(FVector(18.f, 0.f, 15.f));
	PupilMeshComponent->SetRelativeScale3D(FVector(0.725f));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(MainStaticMeshComponent);
	CameraComponent->SetRelativeLocation(FVector(70.f, 0.f, 50.f));
	CameraComponent->SetIsReplicated(true);

	SphereMask = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Mask"));
	SphereMask->SetupAttachment(CameraComponent);
	SphereMask->SetRelativeLocation(FVector(120.f, 0.f, 0.f));
	SphereMask->SetCollisionProfileName("SphereMask");
	SphereMask->SetSphereRadius(6.f);


	bIsRightMouseButtonDown = false;
}

// Called when the game starts or when spawned
void ABGPawn::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);
}

void ABGPawn::OnConstruction(const FTransform& Transform)
{
	if (!SphereMask || !ParameterCollection) return;
	SphereMask->SetRelativeScale3D(FVector(MaskRadius) / FVector(5));

	if (ParameterCollection)
	{
		UKismetMaterialLibrary::SetVectorParameterValue(this, ParameterCollection, VectorParameterName,
		                                                SphereMask->GetComponentLocation());
		UKismetMaterialLibrary::SetScalarParameterValue(this, ParameterCollection, ScalarParameterName, MaskRadius);
	}
}

// Called every frame
void ABGPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/** map the MaskRadius to the SphereMask size (200 / 6 = 33.3333 repeating) */
	if (!SphereMask || !ParameterCollection) return;
	SphereMask->SetSphereRadius(MaskRadius / 33.f);
	UKismetMaterialLibrary::SetVectorParameterValue(this, ParameterCollection, VectorParameterName,
	                                                SphereMask->GetComponentLocation());
	UKismetMaterialLibrary::SetScalarParameterValue(this, ParameterCollection, ScalarParameterName, MaskRadius);
}

// Called to bind functionality to input
void ABGPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	/** Movement */
	InputComponent->BindAxis("MoveForward", this, &ABGPawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABGPawn::MoveRight);

	/** Camera */
	InputComponent->BindAxis("LookUp", this, &ABGPawn::LookUp);
	InputComponent->BindAxis("Turn", this, &ABGPawn::Turn);
}

void ABGPawn::MoveForward(float Value)
{
	if (!CameraComponent) return;

	if (Value != 0.f)
	{
		AddMovementInput(CameraComponent->GetForwardVector(), Value, true);
		UpdateTransform(GetActorTransform());
	}
}

void ABGPawn::MoveRight(float Value)
{
	if (!CameraComponent) return;

	if (Value != 0.f)
	{
		AddMovementInput(CameraComponent->GetRightVector(), Value, true);
		UpdateTransform(GetActorTransform());
	}
}

void ABGPawn::LookUp(float Value)
{
	if (!Controller) return;

	if (Value != 0.f)
	{
		SetActorRelativeRotation(GetActorRotation() + FRotator(-1.f * Value, 0.f, 0.f));
		UpdateTransform(GetActorTransform());
	}
}

void ABGPawn::Turn(float Value)
{
	if (!Controller) return;

	if (Value != 0.f)
	{
		SetActorRelativeRotation(GetActorRotation() + FRotator(0.f, Value, 0.f));
		UpdateTransform(GetActorTransform());
	}
}

void ABGPawn::UpdateTransform(FTransform const& NewTransform) const
{
	if (!Controller) return;

	auto BGPlayerController = Cast<ABGGamePlayerController>(GetController());
	if (BGPlayerController)
	{
		BGPlayerController->UpdateTransformOnServer(NewTransform);
	}
}
