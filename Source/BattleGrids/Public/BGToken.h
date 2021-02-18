// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"

#include "BGToken.generated.h"

class ABGPlayerController;

UCLASS()
class BATTLEGRIDS_API ABGToken : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABGToken();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Multicast, toggles whether or not the token position and rotation is locked.
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "BGToken|Functions")
	void ToggleLockTokenInPlace(bool bLock);

	// Returns the locked state of the token.
	UFUNCTION(BlueprintCallable, Category = "BGToken|Functions")
	bool GetIsTokenLocked() const;

	// Multicast, toggles physics and collision on the token
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "BGToken|Functions")
	void ToggleTokenPhysicsAndCollision(bool const bPhysicsOn, bool const bGravityOn,
	                                    ECollisionEnabled::Type const CollisionType);

	UFUNCTION(BlueprintCallable, Category = "BGToken|Functions")
	bool PlayerControllerHasPermissions(ABGPlayerController const* PlayerController);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGToken|Config")
	FText TokenName;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGToken|Config")
	TArray<ABGPlayerController*> PlayerControllerPermissions;
};
