// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/BGPlayerController.h"
#include "Core/BGTypes.h"

#include "BGGamePlayerController.generated.h"

class UBGSplineWallComponent;
class ABGActor;
class ABGBoard;
class ABGDoor;
class ABGPlayerState;
class ABGSplineStructure;
class ABGStructure;
class ABGTile;
class ABGToken;

/**
 * 
 */
UCLASS()
class BATTLEGRIDS_API ABGGamePlayerController : public ABGPlayerController
{
	GENERATED_BODY()

public:
	ABGGamePlayerController();

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "BGGamePlayerController|UI")
	void SetupGameUI();

	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "BGGamePlayerController|ActorMovement")
	void UpdateTransformOnServer(FTransform const& NewTransform);

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupInputComponent() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Gameplay Data")
	void GetRowNamesOfObjectTypeFromGameMode(EBGActorType const& ObjectType);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Gameplay Data")
	void GetRowNamesOfObjectTypeFromGameMode_Server(EBGActorType const& ObjectType);

	/**
	 * Control Functions
	 */

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Control")
	void SelectObject();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Control")
	void LoadObjectType();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Control")
	void ReleaseObject();

	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "BGGamePlayerController|Control")
	void ToggleContextMenu();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Control")
	bool GetGameMasterPermissions() const;

	/**
	 * Renders an outline around an Actor beneath
	 * the mouse cursor using Custom Render Depth and a Post-Process Material
	 */
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Control")
	void OutlineObject();

	/**
	 * Token Functions
	 */

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

	/**
	 * Spline Structure Functions
	 */

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void HandleSplineStructureSelection();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void SpawnSplineStructureAtLocation(FName const& WallStaticMeshName,
	                                    FName const& WallMaskedMaterialInstanceName,
	                                    FName const& CornerStaticMeshName,
	                                    FName const& CornerMaskedMaterialInstanceName,
	                                    FName const& BaseStaticMeshName,
	                                    FName const& BaseMaterialInstanceName,
	                                    FVector const& Location);

	/**
	 * Modifies a spline by getting the nearest Spline Point
	 * and settings its location to the mouse's location, snapped to grid
	 */
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void ModifySplineStructureLength();


	/**
	 * Adds a new spline point to a spline by
	 * finding the nearest index to the mouse's location and inserts it
	 */
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void AddSplinePointToSplineStructure(UBGSplineWallComponent* InSplineComponent);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void MoveSplineStructure();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void ModifySplineStructureInstanceMeshAtIndex(ABGSplineStructure* StructureToModify, int const& Index,
	                                              FString const& NewInstanceName,
	                                              UStaticMesh* StaticMesh,
	                                              UMaterialInstance* MaterialInstance,
	                                              FString const& OldInstanceName = "WallInstance");

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void SpawnStructureActorAtSplineStructureIndex(ABGSplineStructure* StructureToModify, int const& Index,
	                                               TSubclassOf<ABGStructure> StructureClassToSpawn,
	                                               FString const& OldInstanceName = "WallInstance");

	/**
	 * Removes an Instance at the Index provided on a Spline Structure
	 * By default, removes from the WallInstance
	 */
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void RemoveInstanceAtIndexOnSplineStructure(ABGSplineStructure* StructureToModify, int const& Index,
	                                            FString const& InstanceName = "WallInstance");

	/**
	 * Restores an Instance on a Spline Structure
	 * By default, restores on the WallInstance
	 */
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void RestoreInstanceAtIndexOnSplineStructure(ABGSplineStructure* StructureToModify, int const& Index,
	                                             FTransform const& NewInstanceTransform,
	                                             FString const& InstanceName = "WallInstance");

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void ToggleLockSplineStructureInPlace(ABGSplineStructure* SplineStructureToLock, bool const bNewLocked);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void ResetSplineStructure(ABGSplineStructure* SplineStructureToReset) const;

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void DestroySplineStructure(ABGSplineStructure* SplineStructureToDestroy);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	float GetClosestKeyOnSplineAtMousePosition(UBGSplineWallComponent* SplineComponent, FVector& OutIntersection) const;

	/**
	 * Structure Functions
	 */

	// Destroys a Structure Actor
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void DestroyStructureActor(ABGStructure* StructureToRemove);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void ToggleLockStructure(ABGStructure* StructureToLock, bool const bNewLocked);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Structure")
	void ToggleDoorOpenClose(ABGDoor* DoorToToggle);

	/**
	 * Board Functions
	 */

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Board")
	void HandleActorSelection();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Board")
	void ToggleTileVisibility(ABGTile* TileToToggle);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Board")
	void ShrinkBoard(ABGBoard* BoardToShrink);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Board")
	void GrowBoard(ABGBoard* BoardToGrow);

	/**
	 * BGActor Functions
	 */

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Board")
	void MoveActorToLocation(FVector const& Location);

	////////////////////////
	/// NETWORK Functions

	/**
	 * Network Token Functions
	 */

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Token|Network")
	void SpawnTokenAtLocation_Server(FVector const& Location, FName const& MeshName, FName const& MaterialName);

	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "BGGamePlayerController|Token|Network")
	void MoveTokenToLocation_Server(ABGToken* TokenToMove, FVector const& Location,
	                                FRotator const TokenRotation);

	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "BGGamePlayerController|Token|Network")
	void RotateToken_Server(ABGToken* TokenToRotate, FRotator const& NewRotation);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Token|Network")
	void ResetTokenRotation_Server(ABGToken* TokenToReset);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Token|Network")
	void ToggleTokenLockInPlace_Server(ABGToken* TokenToToggle, bool bLock);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Token|Network")
	void ToggleTokenPermissionsForPlayer_Server(ABGPlayerState* PlayerStateToToggle, ABGToken* TokenToToggle);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Token|Network")
	void DestroyToken_Server(ABGToken* TokenToDestroy);

	/**
	 * Network Spline Structure Functions
	 */

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void SpawnSplineStructureAtLocation_Server(FName const& WallStaticMeshName,
	                                           FName const& WallMaskedMaterialInstanceName,
	                                           FName const& CornerStaticMeshName,
	                                           FName const& CornerMaskedMaterialInstanceName,
	                                           FName const& BaseStaticMeshName,
	                                           FName const& BaseMaterialInstanceName,
	                                           FVector const& Location);

	// Asks the GameMode to make a modification to the GrabbedStructure reference
	UFUNCTION(Server, Unreliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void ModifyStructureLength_Server(UBGSplineWallComponent* InSplineComponent, int const& PointIndex,
	                                  FVector const& NewLocation);

	// Asks the GameMode to add a new spline point to the GrabbedStructure reference
	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void AddSplinePointToSplineStructure_Server(UBGSplineWallComponent* InSplineComponent,
	                                            FVector const& ClickLocation,
	                                            int const& Index);

	UFUNCTION(Server, Unreliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void MoveSplineStructure_Server(ABGSplineStructure* SplineStructureToMove, FVector const& Location);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void ModifyInstanceMeshAtIndex_Server(ABGSplineStructure* SplineStructureToModify, int const& Index,
	                                      FString const& NewInstanceName,
	                                      UStaticMesh* StaticMesh,
	                                      UMaterialInstance* MaterialInstance,
	                                      FString const& OldInstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void SpawnStructureActorAtSplineStructureIndex_Server(ABGSplineStructure* SplineStructureToModify, int const& Index,
	                                                      TSubclassOf<ABGStructure> StructureClassToSpawn,
	                                                      FString const& OldInstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void RemoveInstanceAtIndexOnSplineStructure_Server(ABGSplineStructure* SplineStructureToModify, int const& Index,
	                                                   FString const& InstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void RestoreInstanceAtIndexOnSplineStructure_Server(ABGSplineStructure* SplineStructureToModify, int const& Index,
	                                                    FTransform const& NewInstanceTransform,
	                                                    FString const& InstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void ToggleLockSplineStructureInPlace_Server(ABGSplineStructure* SplineStructureToLock, bool const bNewLocked);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void ResetSplineStructure_Server(ABGSplineStructure* SplineStructureToReset);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void DestroySplineStructure_Server(ABGSplineStructure* SplineStructureToDestroy);

	/**
	 * Network Structure Functions
	 */

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Structures|Network")
	void DestroyStructureActor_Server(ABGStructure* StructureToRemove);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Structures|Network")
	void ToggleLockStructure_Server(ABGStructure* StructureToLock, bool const bNewLocked);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Structures|Network")
	void ToggleDoorOpenClose_Server(ABGDoor* DoorToToggle);

	/**
	 * Network Board & Tile Functions
	 */

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Board|Network")
	void SpawnNewBoard_Server(int const& Zed, int const& X, int const& Y);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Board|Network")
	void ToggleTileVisibility_Server(ABGTile* TileToToggle);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Board|Network")
	void ShrinkBoard_Server(ABGBoard* BoardToShrink);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Board|Network")
	void GrowBoard_Server(ABGBoard* BoardToGrow);

	/**
	 * Network BGActor Functions
	 */

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Actors|Network")
	void MoveActorToLocation_Server(ABGActor* ActorToMove, FVector const& NewLocation);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Actors|Network")
	void SpawnNewActor_Server(TSubclassOf<ABGActor> ActorToSpawn);

	////////////////////////
	/// Callback Functions

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|UI")
	void ShowInGamePlayerListMenu();

	///////////////////////
	/// Variables

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	EBGControlMode ControlMode{EBGControlMode::Build};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	EBGActorType GrabbedObject{EBGActorType::None};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	ABGToken* GrabbedToken{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	ABGSplineStructure* GrabbedStructure{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	ABGActor* GrabbedActor{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	AActor* CurrentOutlinedActor{};

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	TArray<FName> TokenNames;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	TArray<FName> StructureNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	int NearestIndexToClick{-1};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	ABGActor* LastClickedActor{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	FHitResult LastHitResult{};
};
