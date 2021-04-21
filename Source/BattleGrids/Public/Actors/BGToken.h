// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


#include "Components/WidgetComponent.h"
#include "Core/BGActorInterface.h"
#include "GameFramework/Character.h"

#include "BGToken.generated.h"

class AAIController;
class ABGPlayerState;
class UCapsuleComponent;
class UCharacterMovementComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UWidgetComponent;
class UBGContextMenu;

/**
* BattleGrids Token class to be used for miniatures and models on the gameboard
*/
UCLASS()
class BATTLEGRIDS_API ABGToken : public ACharacter, public IBGActorInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABGToken();

	// Call when spawning Token via DeferredActorSpawn function to set the mesh and material
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "BGToken|Functions")
	void InitializeMeshAndMaterial(UStaticMesh* ModelStaticMesh, UMaterialInstance* MaterialInstance,
	                               UStaticMesh* BaseStaticMesh) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Multicast, toggles whether or not the token position and rotation is locked.
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "BGToken|Functions")
	void ToggleLockTokenInPlace(bool const bLock);

	// Returns the locked state of the token.
	UFUNCTION(BlueprintCallable, Category = "BGToken|Functions")
	bool GetIsTokenLocked() const;

	// Checks whether or not a PlayerState exists in the Token's permissions array
	UFUNCTION(BlueprintCallable, Category = "BGToken|Functions")
	bool PlayerHasPermissions(ABGPlayerState const* PlayerStateToCheck);

	UFUNCTION(BlueprintCallable, Category = "BGToken|Functions")
	bool AddPlayerToPermissionsArray(ABGPlayerState* PlayerStateToAdd);

	UFUNCTION(BlueprintCallable, Category = "BGToken|Functions")
	bool RemovePlayerFromPermissionsArray(ABGPlayerState* PlayerStateToRemove);

	//////////////////////
	/// Getters

	UStaticMeshComponent* GetTokenModelStaticMeshComponent() const { return TokenModelStaticMeshComponent; }
	UStaticMeshComponent* GetTokenBaseStaticMeshComponent() const { return TokenBaseStaticMeshComponent; }

	AAIController* GetCharacterAIController() const { return CharacterAIController; }

	TArray<ABGPlayerState*> GetPlayerPermissions() const { return PlayerPermissions; }

	//////////////////////
	/// Setters

	void SetWidgetComponentClass(TSubclassOf<UUserWidget> const& InClass) const;

	//////////////////////
	/// Interface Implementation

	virtual void ToggleContextMenu() override;
	virtual void CloseContextMenu() override;
	virtual UBGContextMenu* GetContextMenu() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void FellOutOfWorld(const UDamageType& dmgType) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TokenBaseStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TokenModelStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* ContextMenuWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGToken|Config")
	FText TokenName;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGToken|Config")
	TArray<ABGPlayerState*> PlayerPermissions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
	AAIController* CharacterAIController;
};
