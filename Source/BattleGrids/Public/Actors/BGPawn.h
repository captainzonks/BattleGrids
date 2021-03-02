// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BGPawn.generated.h"

UCLASS()
class BATTLEGRIDS_API ABGPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABGPawn();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Category = "ABGPawn|Movement")
	void UpdateTransform(FTransform NewTransform);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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
	class UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UPawnMovementComponent* PawnMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UFloatingPawnMovement* FloatingPawnMovement;

	uint8 bIsRightMouseButtonDown : 1;
};
