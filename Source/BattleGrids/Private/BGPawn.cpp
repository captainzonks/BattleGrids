// Fill out your copyright notice in the Description page of Project Settings.


#include "BGPawn.h"


#include "BGPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"

// Sets default values
ABGPawn::ABGPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Component"));
	RootComponent = CapsuleComponent;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMeshComponent->SetupAttachment(CapsuleComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(StaticMeshComponent);
	CameraComponent->SetRelativeLocation(FVector(50.f, 0.f, 0.f));
	CameraComponent->SetIsReplicated(true);

	PawnMovementComponent = CreateDefaultSubobject<UPawnMovementComponent>(TEXT("Pawn Movement"));

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(
		TEXT("Floating Pawn Movement"));
	FloatingPawnMovement->SetIsReplicated(true);

	bIsRightMouseButtonDown = false;
}

// Called when the game starts or when spawned
void ABGPawn::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);
}

// Called every frame
void ABGPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABGPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Movement
	InputComponent->BindAxis("MoveForward", this, &ABGPawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABGPawn::MoveRight);

	// Camera
	InputComponent->BindAxis("LookUp", this, &ABGPawn::LookUp);
	InputComponent->BindAxis("Turn", this, &ABGPawn::Turn);
}

void ABGPawn::MoveForward(float Value)
{
	if (Value != 0.f)
	{
		AddMovementInput(CameraComponent->GetForwardVector(), Value);
	}
	UpdateTransform(GetActorTransform());
}

void ABGPawn::MoveRight(float Value)
{
	if (Value != 0.f)
	{
		AddMovementInput(CameraComponent->GetRightVector(), Value);
	}
	UpdateTransform(GetActorTransform());
}

void ABGPawn::LookUp(float Value)
{
	if (Value != 0.f)
	{
		// StaticMeshComponent->AddRelativeRotation(FRotator(-1.f * Value, 0.f, 0.f));
		SetActorRelativeRotation(GetActorRotation() + FRotator(-1.f * Value, 0.f, 0.f));
	}
	UpdateTransform(GetActorTransform());
}

void ABGPawn::Turn(float Value)
{
	if (Value != 0.f)
	{
		// StaticMeshComponent->AddRelativeRotation(FRotator(0.f, Value, 0.f));
		SetActorRelativeRotation(GetActorRotation() + FRotator(0.f, Value, 0.f));

	}
	UpdateTransform(GetActorTransform());
}

void ABGPawn::UpdateTransform(FTransform NewTransform)
{
	Cast<ABGPlayerController>(Controller)->UpdateTransformOnServer(NewTransform);
}
