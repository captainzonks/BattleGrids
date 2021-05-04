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
class ABGCharacter;

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
	void GetRowNamesOfObjectTypeFromGameMode(EBGClassCategory const& ClassCategory);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Gameplay Data")
	void GetRowNamesOfObjectTypeFromGameMode_Server(EBGClassCategory const& ClassCategory);

	/**
	 * Control Functions
	 */

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Control")
	void SelectActor();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Control")
	void LoadGrabbedActor();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Control")
	void ReleaseGrabbedActor();

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
	 * Character Functions
	 */

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Character")
	void MoveCharacterToLocation();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Character")
	void RotateCharacter(float Value);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Character")
	void ResetCharacterRotation(ABGCharacter* CharacterToReset);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Character")
	void ToggleCharacterLockInPlace(ABGCharacter* CharacterToToggle, bool bLock);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Character")
	void ToggleCharacterPermissionsForPlayer(ABGPlayerState* PlayerStateToToggle, ABGCharacter* CharacterToToggle);

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
	void ModifySplineStructureInstanceMeshAtIndex(UBGSplineWallComponent* InSplineComponent, int const& Index,
	                                              FString const& NewInstanceName,
	                                              UStaticMesh* StaticMesh,
	                                              UMaterialInstance* MaterialInstance,
	                                              FString const& OldInstanceName = "WallInstance");

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void SpawnStructureActorAtSplineStructureIndex(UBGSplineWallComponent* InSplineComponent, int const& Index,
	                                               TSubclassOf<ABGStructure> StructureClassToSpawn,
	                                               FString const& OldInstanceName = "WallInstance");

	/**
	 * Removes an Instance at the Index provided on a Spline Structure
	 * By default, removes from the WallInstance
	 */
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void RemoveInstanceAtIndexOnSplineStructure(UBGSplineWallComponent* InSplineComponent, int const& Index,
	                                            FString const& InstanceName = "WallInstance");

	/**
	 * Restores an Instance on a Spline Structure
	 * By default, restores on the WallInstance
	 */
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void RestoreInstanceAtIndexOnSplineStructure(UBGSplineWallComponent* InSplineComponent, int const& Index,
	                                             FTransform const& NewInstanceTransform,
	                                             FString const& InstanceName = "WallInstance");

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void ToggleLockSplineStructureInPlace(ABGSplineStructure* SplineStructureToLock, bool const bNewLocked);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	void ResetSplineStructure(UBGSplineWallComponent* InSplineComponent) const;

	// UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|SplineStructure")
	// void DestroySplineStructure(ABGSplineStructure* SplineStructureToDestroy);

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
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Gameplay")
	void DestroyGameActor(AActor* ActorToDestroy);
	
	/**
	 * Network Token Functions
	 */

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Character|Network")
	void SpawnCharacterAtLocation_Server(FVector const& Location, FName const& MeshName, FName const& MaterialName);

	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "BGGamePlayerController|Character|Network")
	void MoveCharacterToLocation_Server(ABGCharacter* CharacterToMove, FVector const& Location,
	                                FRotator const TokenRotation);

	UFUNCTION(Server, Unreliable, BlueprintCallable, Category = "BGGamePlayerController|Character|Network")
	void RotateCharacter_Server(ABGCharacter* CharacterToRotate, FRotator const& NewRotation);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Character|Network")
	void ResetCharacterRotation_Server(ABGCharacter* CharacterToReset);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Character|Network")
	void ToggleCharacterLockInPlace_Server(ABGCharacter* CharacterToToggle, bool bLock);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Character|Network")
	void ToggleCharacterPermissionsForPlayer_Server(ABGPlayerState* PlayerStateToToggle, ABGCharacter* CharacterToToggle);

	// UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BGGamePlayerController|Token|Network")
	// void DestroyToken_Server(AActor* TokenToDestroy);

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

	// UFUNCTION(Server, Unreliable, Category = "BGGamePlayerController|SplineStructure|Network")
	// void MoveSplineStructure_Server(ABGSplineStructure* SplineStructureToMove, FVector const& Location);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void ModifyInstanceMeshAtIndex_Server(UBGSplineWallComponent* InSplineComponent, int const& Index,
	                                      FString const& NewInstanceName,
	                                      UStaticMesh* StaticMesh,
	                                      UMaterialInstance* MaterialInstance,
	                                      FString const& OldInstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void SpawnStructureActorAtSplineStructureIndex_Server(UBGSplineWallComponent* InSplineComponent, int const& Index,
	                                                      TSubclassOf<ABGStructure> StructureClassToSpawn,
	                                                      FString const& OldInstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void RemoveInstanceAtIndexOnSplineStructure_Server(UBGSplineWallComponent* InSplineComponent, int const& Index,
	                                                   FString const& InstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void RestoreInstanceAtIndexOnSplineStructure_Server(UBGSplineWallComponent* InSplineComponent, int const& Index,
	                                                    FTransform const& NewInstanceTransform,
	                                                    FString const& InstanceName = "WallInstance");

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void ToggleLockSplineStructureInPlace_Server(ABGSplineStructure* SplineStructureToLock, bool const bNewLocked);

	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|SplineStructure|Network")
	void ResetSplineStructure_Server(UBGSplineWallComponent* InSplineComponent);

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
	void MoveActorToLocation_Server(AActor* ActorToMove, FVector const& NewLocation);

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
	FHitResult LastHitResult{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	AActor* LastClickedActor{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	AActor* GrabbedActor{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	EBGActorType GrabbedActorType{EBGActorType::None};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	AActor* CurrentOutlinedActor{};

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	TArray<FName> TokenNames;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	TArray<FName> ActorNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	int NearestIndexToClick{-1};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	UBGGameInstance* BGGameInstance;

};
