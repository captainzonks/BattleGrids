// © 2021 Matthew Barham. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/BGPlayerController.h"
#include "Core/BGTypes.h"

#include "BGGamePlayerController.generated.h"

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

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Input")
	void SelectObject();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Input")
	void ReleaseObject();

	////////////////////////
	/// Token Functions

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Input")
	void HandleTokenSelection();

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Input")
	void MoveTokenToLocation(bool const bHolding);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Input")
	void RotateToken(float Value);

	////////////////////////
	/// Structural Functions

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Input")
	void HandleStructureSelection();

	/* Modifies a spline by getting the nearest Spline Point and settings its location to the mouse's location, snapped to grid */
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Input")
	void ModifyStructureLength();

	/* Adds a new spline point to a spline by finding the nearest index to the mouse's location and inserts it */
	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Input")
	void AddSplinePointToStructureSpline();


	////////////////////////
	/// NETWORK Functions

	/// Tokens
	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void SetTokenCollisionAndPhysics_Server(ABGToken* TokenToModify, bool const bPhysicsOn, bool const bGravityOn,
	                                        ECollisionEnabled::Type const CollisionType);

	UFUNCTION(Server, Unreliable, Category = "BGGamePlayerController|Network")
	void MoveTokenToLocation_Server(ABGToken* TokenToMove, AActor* TargetActor, bool const bHolding,
	                                FRotator const TokenRotation);

	/// Structures

	/* Asks the GameMode to make a modification to the GrabbedStructure reference */
	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void ModifyStructureLength_Server(ABGSplineStructure* StructureToModify, int const& PointIndex,
	                                  FVector const& NewLocation);

	/* Asks the GameMode to add a new spline point to the GrabbedStructure reference */
	UFUNCTION(Server, Reliable, Category = "BGGamePlayerController|Network")
	void AddSplinePointToStructureSpline_Server(ABGSplineStructure* StructureToModify, FVector const& ClickLocation,
	                                            int const& Index);

	UFUNCTION(BlueprintCallable, Category = "BGGamePlayerController|Permissions")
	bool GetGameMasterPermissions() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	EBGControlMode ControlMode{EBGControlMode::Move};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	EBGGrabbedObjectType GrabbedObject{EBGGrabbedObjectType::None};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	class ABGToken* GrabbedToken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	class ABGSplineStructure* GrabbedStructure;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BGGamePlayerController|Config")
	class ABGBoard* GrabbedBoard;

	//////////////////////////////////////////
	/// Variables (Not Exposed to Blueprints)

	UPROPERTY()
	int NearestIndexToClick{-1};

	UPROPERTY()
	AActor* LastTargetedActor{};
};
