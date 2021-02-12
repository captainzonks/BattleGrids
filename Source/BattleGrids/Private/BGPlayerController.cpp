// Fill out your copyright notice in the Description page of Project Settings.


#include "BGPlayerController.h"

#include "BGPawn.h"

void ABGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();


}

void ABGPlayerController::UpdateTransformOnServer_Implementation(FTransform NewTransform)
{
	GetPawn()->SetActorTransform(NewTransform);
	UE_LOG(LogTemp, Warning, TEXT("Update Transform on Server Called"))
}