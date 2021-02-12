// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BGPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BATTLEGRIDS_API ABGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGPlayerController|Movement")
	void UpdateTransformOnServer(FTransform NewTransform);
};
