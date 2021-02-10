// Fill out your copyright notice in the Description page of Project Settings.


#include "BGPlayerController.h"

#include "BGPawn.h"
#include "Camera/CameraComponent.h"

void ABGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();


}

void ABGPlayerController::UpdateTransformOnServer_Implementation(FTransform NewTransform)
{
	GetPawn()->SetActorTransform(NewTransform);
	UE_LOG(LogTemp, Warning, TEXT("Update Transform on Server Called"))
}


// void ABGPlayerController::MoveForward_Server_Implementation(float Value)
// {
// 	if (GetPawn())
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("We have a pawn"))
// 	}
// 	GetPawn()->AddMovementInput(Cast<ABGPawn>(GetPawn())->GetCameraComponent()->GetForwardVector(), Value);
// }
//
// void ABGPlayerController::MoveRight_Server_Implementation(float Value)
// {
// 	GetPawn()->AddMovementInput(Cast<ABGPawn>(GetPawn())->GetCameraComponent()->GetRightVector(), Value);
// }

// void ABGPlayerController::ServerSetTransform_Implementation(FTransform Transform)
// {
// 	SetActorTransform(Transform);
// }


