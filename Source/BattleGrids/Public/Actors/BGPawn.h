// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BGPawn.generated.h"

UCLASS()
class BATTLEGRIDS_API ABGPawn : public APawn
{
	GENERATED_BODY()

public:
	/** Default UObject constructor. */
	ABGPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** Allows a Pawn to set up custom input bindings. Called upon possession by a PlayerController, using the InputComponent created by CreatePlayerInputComponent(). */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Category = "ABGPawn|Movement")
	void UpdateTransform(FTransform const& NewTransform) const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	UFUNCTION(BlueprintCallable, Category = "ABGPawn|Movement")
	void MoveForward(float Value);

	UFUNCTION(BlueprintCallable, Category = "ABGPawn|Movement")
	void MoveRight(float Value);

	UFUNCTION(BlueprintCallable, Category = "ABGPawn|Camera")
	void LookUp(float Value);

	UFUNCTION(BlueprintCallable, Category = "ABGPawn|Camera")
	void Turn(float Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MainStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* PupilMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* SphereMask;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UPawnMovementComponent* PawnMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UFloatingPawnMovement* FloatingPawnMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float MaskRadius{200.f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	class UMaterialParameterCollection* ParameterCollection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	FName VectorParameterName{"PrimeLocation"};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Config")
	FName ScalarParameterName{"Radius"};

	uint8 bIsRightMouseButtonDown : 1;
};