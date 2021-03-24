// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


#include "BGGameplayGameModeBase.h"
#include "Core/BGPlayerController.h"
#include "Core/BGTypes.h"

#include "BGGamePlayerController.generated.h"

class ABGBoard;
class ABGPlayerState;
class ABGStructure;
class ABGTile;

/**
 * 
 */
UCLASS()
class BATTLEGRIDS_API ABGGamePlayerController : public ABGPlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "BGGamePlayerController|ActorMovement")
	void UpdateTransformOnServer(FTransform NewTransform);

protected:
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupInputComponent() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Gameplay Data")
	void GetRowNamesOfObjectTypeFromGameMode(EBGObjectType const& ObjectType);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Gameplay Data")
	void GetRowNamesOfObjectTypeFromGameMode_Server(EBGObjectType const& ObjectType);

	/////////////////////////
	/// Control Functions

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Control")
	void SelectObject();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Control")
	void ReleaseObject();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Control")
	bool GetGameMasterPermissions() const;

	////////////////////////
	/// Token Functions

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Token")
	void HandleTokenSelection();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Token")
	void SetTokenCollisionAndPhysics(ABGToken* TokenToModify, bool const bPhysicsOn, bool const bGravityOn,
	                                 ECollisionEnabled::Type const CollisionType);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Token")
	void MoveTokenToLocation(bool const bHolding);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Token")
	void RotateToken(float Value);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Token")
	void ResetTokenRotation(ABGToken* TokenToReset);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Token")
	void ToggleTokenLockInPlace(ABGToken* TokenToToggle, bool bLock);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Token")
	void ToggleTokenPermissionsForPlayer(ABGPlayerState* PlayerStateToToggle, ABGToken* TokenToToggle);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Token")
	void DestroyToken(ABGToken* TokenToDestroy);

	////////////////////////
	/// Structure Functions

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void HandleSplineStructureSelection();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void SpawnSplineStructureAtLocation(FVector const& Location, FName const& WallStaticMeshName,
	                                    FName const& WallMaskedMaterialInstanceName,
	                                    FName const& CornerStaticMeshName,
	                                    FName const& CornerMaskedMaterialInstanceName,
	                                    FName const& BaseStaticMeshName,
	                                    FName const& BaseMaterialInstanceName);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void SetSplineStructurePhysicsAndCollision(ABGSplineStructure* StructureToModify, bool const bGravityOn,
	                                           ECollisionEnabled::Type const CollisionType);

	// Modifies a spline by getting the nearest Spline Point and settings its location to the mouse's location, snapped to grid
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void ModifySplineStructureLength();

	// Adds a new spline point to a spline by finding the nearest index to the mouse's location and inserts it
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void AddSplinePointToSplineStructure();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void MoveSplineStructure();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void ModifySplineStructureInstanceMeshAtIndex(ABGSplineStructure* StructureToModify, int const& Index,
	                                              FString const& NewInstanceName,
	                                              UStaticMesh* StaticMesh,
	                                              UMaterialInstance* MaterialInstance,
	                                              FString const& OldInstanceName = "WallInstance");

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void SpawnStructureActorAtSplineStructureIndex(ABGSplineStructure* StructureToModify, int const& Index,
	                                               TSubclassOf<ABGStructure> StructureClassToSpawn,
	                                               FString const& OldInstanceName = "WallInstance");

	// Removes an Instance at the Index provided on a Spline Structure
	// By default, removes from the WallInstance
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void RemoveInstanceAtIndexOnSplineStructure(ABGSplineStructure* StructureToModify, int const& Index,
	                                            FString const& InstanceName = "WallInstance");

	// Restores an Instance on a Spline Structure
	// By default, restores on the WallInstance
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Network")
	void RestoreInstanceAtIndexOnSplineStructure(ABGSplineStructure* StructureToModify,
	                                             FTransform const& NewInstanceTransform,
	                                             FString const& InstanceName = "WallInstance");

	// Destroys a Structure Actor
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void DestroyStructureActor(ABGStructure* StructureToRemove);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void ToggleLockSplineStructureInPlace(ABGSplineStructure* SplineStructureToLock, bool const bNewLocked);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void ToggleLockStructure(ABGStructure* StructureToLock, bool const bNewLocked);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void ResetSplineStructure(ABGSplineStructure* SplineStructureToReset) const;

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void DestroySplineStructure(ABGSplineStructure* SplineStructureToDestroy);

	////////////////////////
	/// Board Functions

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Board")
	void HandleBoardSelection();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Board")
	void MoveBoardToLocation(FVector const& Location);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Board")
	void ToggleTileVisibility(ABGTile* TileToToggle);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Board")
	void ShrinkBoard(ABGBoard* BoardToShrink);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Board")
	void GrowBoard(ABGBoard* BoardToGrow);

	////////////////////////
	/// NETWORK Functions

	/// Tokens

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void SpawnTokenAtLocation_Server(FVector const& Location, FName const& MeshName, FName const& MaterialName);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void SetTokenCollisionAndPhysics_Server(ABGToken* TokenToModify, bool const bPhysicsOn, bool const bGravityOn,
	                                        ECollisionEnabled::Type const CollisionType);

	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void MoveTokenToLocation_Server(ABGToken* TokenToMove, FVector const& Location,
	                                FRotator const TokenRotation);

	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void RotateToken_Server(ABGToken* TokenToRotate, FRotator const& NewRotation);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void ResetTokenRotation_Server(ABGToken* TokenToReset);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void ToggleTokenLockInPlace_Server(ABGToken* TokenToToggle, bool bLock);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void ToggleTokenPermissionsForPlayer_Server(ABGPlayerState* PlayerStateToToggle, ABGToken* TokenToToggle);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void DestroyToken_Server(ABGToken* TokenToDestroy);

	/// Structures

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void SpawnSplineStructureAtLocation_Server(FVector const& Location, FName const& WallStaticMeshName,
	                                           FName const& WallMaskedMaterialInstanceName,
	                                           FName const& CornerStaticMeshName,
	                                           FName const& CornerMaskedMaterialInstanceName,
	                                           FName const& BaseStaticMeshName,
	                                           FName const& BaseMaterialInstanceName);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void SetStructurePhysicsAndCollision_Server(ABGSplineStructure* SplineStructureToModify, bool const bGravityOn,
	                                            ECollisionEnabled::Type const CollisionType);

	// Asks the GameMode to make a modification to the GrabbedStructure reference
	UFUNCTION(Server, Unreliable, Category = "BGGamePlayerController|Network")
	void ModifyStructureLength_Server(ABGSplineStructure* SplineStructureToModify, int const& PointIndex,
	                                  FVector const& NewLocation);

	// Asks the GameMode to add a new spline point to the GrabbedStructure reference
	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void AddSplinePointToStructureSpline_Server(ABGSplineStructure* SplineStructureToModify,
	                                            FVector const& ClickLocation,
	                                            int const& Index);

	UFUNCTION(Server, Unreliable, Category = "BGGamePlayerController|Network")
	void MoveStructure_Server(ABGSplineStructure* SplineStructureToMove, FVector const& Location);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void ModifyInstanceMeshAtIndex_Server(ABGSplineStructure* SplineStructureToModify, int const& Index,
	                                      FString const& NewInstanceName,
	                                      UStaticMesh* StaticMesh,
	                                      UMaterialInstance* MaterialInstance,
	                                      FString const& OldInstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void SpawnStructureActorAtSplineStructureIndex_Server(ABGSplineStructure* SplineStructureToModify, int const& Index,
	                                                      TSubclassOf<ABGStructure> StructureClassToSpawn,
	                                                      FString const& OldInstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void RemoveInstanceAtIndexOnSplineStructure_Server(ABGSplineStructure* SplineStructureToModify, int const& Index,
	                                                   FString const& InstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void RestoreInstanceAtIndexOnSplineStructure_Server(ABGSplineStructure* SplineStructureToModify,
	                                                    FTransform const& NewInstanceTransform,
	                                                    FString const& InstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void DestroyStructureActor_Server(ABGStructure* StructureToRemove);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void ToggleLockSplineStructureInPlace_Server(ABGSplineStructure* SplineStructureToLock, bool const bNewLocked);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void ToggleLockStructure_Server(ABGStructure* StructureToLock, bool const bNewLocked);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void ResetSplineStructure_Server(ABGSplineStructure* SplineStructureToReset);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void DestroySplineStructure_Server(ABGSplineStructure* SplineStructureToDestroy);

	/// Boards

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void SpawnNewBoard_Server(int const& Zed, int const& X, int const& Y);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void MoveBoardToLocation_Server(ABGBoard* BoardToMove, FVector const& NewLocation);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void ToggleTileVisibility_Server(ABGTile* TileToToggle);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void ShrinkBoard_Server(ABGBoard* BoardToShrink);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Network")
	void GrowBoard_Server(ABGBoard* BoardToGrow);


	///////////////////////
	/// Variables

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	EBGControlMode ControlMode{EBGControlMode::Move};

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	EBGObjectType GrabbedObject{EBGObjectType::None};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	class ABGToken* GrabbedToken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	class ABGSplineStructure* GrabbedStructure;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	class ABGBoard* GrabbedBoard;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	TArray<FName> TokenNames;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	TArray<FName> StructureNames;

	//////////////////////////////////////////
	/// Variables (Not Exposed to Blueprints)

	UPROPERTY()
	int NearestIndexToClick{-1};

	UPROPERTY()
	AActor* LastTargetedActor{};

	UPROPERTY()
	FHitResult LastHitResult{};
};
