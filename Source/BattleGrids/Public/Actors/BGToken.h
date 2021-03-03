// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"

#include "BGToken.generated.h"

class ABGPlayerState;

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
	void SetTokenPhysicsAndCollision(bool const bPhysicsOn, bool const bGravityOn,
	                                    ECollisionEnabled::Type const CollisionType);

	// Checks whether or not a PlayerState exists in the Token's permissions array
	UFUNCTION(BlueprintCallable, Category = "BGToken|Functions")
	bool PlayerHasPermissions(ABGPlayerState const* PlayerState);

	UFUNCTION(BlueprintCallable, Category = "BGToken|Functions")
	bool AddPlayerToPermissionsArray(ABGPlayerState* PlayerStateToAdd);

	UFUNCTION(BlueprintCallable, Category = "BGToken|Functions")
	bool RemovePlayerFromPermissionsArray(ABGPlayerState* PlayerStateToRemove);

	//////////////////////
	/// Getters
	
	class UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

	TArray<ABGPlayerState*> GetPlayerPermissions() const { return PlayerPermissions; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void FellOutOfWorld(const UDamageType& dmgType) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGToken|Config")
	FText TokenName;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGToken|Config")
	TArray<ABGPlayerState*> PlayerPermissions;
};
