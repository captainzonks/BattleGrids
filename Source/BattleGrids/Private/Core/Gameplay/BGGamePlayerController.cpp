// © 2021 Matthew Barham. All Rights Reserved.

#include "Core/Gameplay/BGGamePlayerController.h"

#include "Actors/BGBoard.h"
#include "Actors/BGSplineStructure.h"
#include "Actors/BGTile.h"
#include "Actors/BGToken.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Core/BGPlayerState.h"
#include "Core/Gameplay/BGGameplayGameModeBase.h"
#include "Engine/DemoNetDriver.h"
#include "Kismet/GameplayStatics.h"

void ABGGamePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	switch (GrabbedObject)
	{
	case EBGObjectType::None: break;
	case EBGObjectType::Token:
		HandleTokenSelection();
		break;
	case EBGObjectType::Structure:
		HandleStructureSelection();
		break;
	case EBGObjectType::Board:
		HandleBoardSelection();
		break;
	default: ;
	}
}

void ABGGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Handle Select (Default: Left Click w/ Mouse)
	InputComponent->BindAction("Select", IE_Pressed, this, &ABGGamePlayerController::SelectObject);
	InputComponent->BindAction("Select", IE_Released, this, &ABGGamePlayerController::ReleaseObject);

	// Token Movement Handling
	InputComponent->BindAxis("RotateToken", this, &ABGGamePlayerController::RotateToken);
}

void ABGGamePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGGamePlayerController, ControlMode)
	DOREPLIFETIME(ABGGamePlayerController, GrabbedObject)
	DOREPLIFETIME(ABGGamePlayerController, TokenNames)
	DOREPLIFETIME(ABGGamePlayerController, StructureNames)
}

void ABGGamePlayerController::GetRowNamesOfObjectTypeFromGameMode(EBGObjectType const& ObjectType)
{
	GetRowNamesOfObjectTypeFromGameMode_Server(ObjectType);
}

void ABGGamePlayerController::GetRowNamesOfObjectTypeFromGameMode_Server_Implementation(
	EBGObjectType const& ObjectType)
{
	switch (ObjectType)
	{
	case EBGObjectType::None: break;
	case EBGObjectType::Token:
		TokenNames = Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this))->GetRowNamesOfType(ObjectType);
		break;
	case EBGObjectType::Structure:
		StructureNames = Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this))->
			GetRowNamesOfType(ObjectType);
		break;
	case EBGObjectType::Board: break;
	default: ;
	}
}

void ABGGamePlayerController::SelectObject()
{
	FHitResult HitResult;
	GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, HitResult);

	if (HitResult.bBlockingHit && HitResult.GetActor()->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("LineTrace Hit Actor: %s"), *HitResult.GetActor()->GetName())

		if ((GrabbedToken = Cast<ABGToken>(HitResult.GetActor()))->IsValidLowLevel())
		{
			GrabbedObject = EBGObjectType::Token;
			return;
		}

		if ((GrabbedStructure = Cast<ABGSplineStructure>(HitResult.GetActor()))->IsValidLowLevel())
		{
			GrabbedObject = EBGObjectType::Structure;
			return;
		}

		if ((GrabbedBoard = Cast<ABGBoard>(HitResult.GetActor()))->IsValidLowLevel())
		{
			GrabbedObject = EBGObjectType::Board;
		}
	}
}

void ABGGamePlayerController::ReleaseObject()
{
	GrabbedObject = EBGObjectType::None;

	if (GrabbedToken)
	{
		MoveTokenToLocation(false);
		SetTokenCollisionAndPhysics(GrabbedToken, true, true, ECollisionEnabled::Type::QueryAndPhysics);
	}

	if (GrabbedStructure)
	{
		SetStructurePhysicsAndCollision(GrabbedStructure, true,
		                                ECollisionEnabled::Type::QueryAndPhysics);
	}

	GrabbedToken = nullptr;
	GrabbedStructure = nullptr;
	GrabbedBoard = nullptr;
	LastTargetedActor = nullptr;
	LastHitResult.Reset();
	NearestIndexToClick = -1;
}

void ABGGamePlayerController::HandleTokenSelection()
{
	if (GrabbedToken && ControlMode == EBGControlMode::Move && !GrabbedToken->GetIsTokenLocked())
	{
		if (!GetGameMasterPermissions() && !GrabbedToken->PlayerHasPermissions(GetPlayerState<ABGPlayerState>()))
			return;

		SetTokenCollisionAndPhysics(GrabbedToken, true, false, ECollisionEnabled::Type::PhysicsOnly);

		LastHitResult.Reset();
		LastTargetedActor = nullptr;
		if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, LastHitResult))
		{
			if (LastHitResult.bBlockingHit && LastHitResult.GetActor()->IsValidLowLevel())
			{
				UE_LOG(LogTemp, Warning, TEXT("Cursor Hit: %s"), *LastHitResult.GetActor()->GetName())

				if (LastHitResult.GetActor()->IsA(ABGTile::StaticClass()))
				{
					UE_LOG(LogTemp, Warning, TEXT("Cursor Hit A Tile"))
					if (!Cast<ABGTile>(LastHitResult.GetActor())->GetStaticMeshComponent()->GetVisibleFlag())
						return;
				}

				LastTargetedActor = LastHitResult.GetActor();
				MoveTokenToLocation(true);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("NO HIT RESULT"))
		}
	}
}

void ABGGamePlayerController::SetTokenCollisionAndPhysics(ABGToken* TokenToModify, bool const bPhysicsOn,
                                                          bool const bGravityOn,
                                                          ECollisionEnabled::Type const CollisionType)
{
	if (TokenToModify)
	{
		if (!HasAuthority())
		{
			TokenToModify->SetTokenPhysicsAndCollision(bPhysicsOn, bGravityOn, CollisionType);
		}
		SetTokenCollisionAndPhysics_Server(TokenToModify, bPhysicsOn, bGravityOn, CollisionType);
	}
}

void ABGGamePlayerController::MoveTokenToLocation(bool const bHolding)
{
	if (GrabbedToken && LastTargetedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveTokenToLocation(): LastTargetedActor = %s"), *LastTargetedActor->GetName())

		FRotator const Rotation = FRotator(0.f, GrabbedToken->GetActorRotation().Yaw, 0.f);

		float ZedValue;
		bHolding ? ZedValue = 100.f : ZedValue = 50.f;

		FVector Location;

		if (Cast<ABGSplineStructure>(LastTargetedActor))
		{
			Location = LastHitResult.GetComponent()->Bounds.Origin + FVector(
				0.f, 0.f, LastHitResult.GetComponent()->Bounds.BoxExtent.Z + 50.f);
		}
		else
		{
			FVector ActorOrigin{};
			FVector ActorBoxExtent{};

			LastTargetedActor->GetActorBounds(false, ActorOrigin, ActorBoxExtent, false);

			Location.X = ActorOrigin.X;
			Location.Y = ActorOrigin.Y;
			Location.Z = ZedValue + ActorOrigin.Z + ActorBoxExtent.Z;
		}

		// Move the token locally if we are a client
		if (!HasAuthority())
		{
			GrabbedToken->SetActorLocation(Location, true, nullptr, ETeleportType::None);
			GrabbedToken->SetActorRotation(Rotation, ETeleportType::None);
		}

		// Make a server call to ask the GameMode to move the token
		MoveTokenToLocation_Server(GrabbedToken, Location, Rotation);
	}
}

void ABGGamePlayerController::HandleStructureSelection()
{
	if (!GetGameMasterPermissions() || GrabbedStructure->GetLockedStatus())
		return;

	switch (ControlMode)
	{
	case EBGControlMode::Build:
		if (GetInputAnalogKeyState(EKeys::LeftAlt) == 1)
		{
			AddSplinePointToStructureSpline();
			break;
		}
		ModifyStructureLength();
		break;
	case EBGControlMode::Edit: break;
	case EBGControlMode::Move:
		MoveStructure();
		break;
	default: ;
	}
}

void ABGGamePlayerController::SetStructurePhysicsAndCollision(ABGSplineStructure* StructureToModify,
                                                              bool const bGravityOn,
                                                              ECollisionEnabled::Type const CollisionType)
{
	if (StructureToModify)
	{
		if (!HasAuthority())
		{
			StructureToModify->SetStructurePhysicsAndCollision(bGravityOn, CollisionType);
		}

		SetStructurePhysicsAndCollision_Server(StructureToModify, bGravityOn, CollisionType);
	}
}

void ABGGamePlayerController::ModifyStructureLength()
{
	if (GrabbedStructure)
	{
		FVector WorldPosition;
		FVector WorldDirection;

		DeprojectMousePositionToWorld(WorldPosition, WorldDirection);

		auto GridSnappedIntersection = FMath::LinePlaneIntersection(WorldPosition,
		                                                            WorldDirection * 2000.f + WorldPosition,
		                                                            GrabbedStructure->GetActorLocation(),
		                                                            FVector(0.f, 0.f, 1.f)).GridSnap(100.f);

		GridSnappedIntersection.Z = GrabbedStructure->GetActorLocation().Z;

		UE_LOG(LogTemp, Warning, TEXT("Number of Instance Components: %i"),
		       GrabbedStructure->GetInstancedStaticMeshComponentByString("WallInstance")->GetInstanceCount())

		if (NearestIndexToClick < 0)
		{
			if (GrabbedStructure->GetInstancedStaticMeshComponentByString("WallInstance")->GetInstanceCount() < 2)
			{
				NearestIndexToClick = 1;
			}
			else
			{
				NearestIndexToClick = FMath::RoundToInt(
					GrabbedStructure->GetSplineComponent()->FindInputKeyClosestToWorldLocation(
						GridSnappedIntersection));
			}
		}


		if (!HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("NearestIndexToClick: %i"), NearestIndexToClick)
			GrabbedStructure->SetLocationOfSplinePoint(NearestIndexToClick, GridSnappedIntersection);
			GrabbedStructure->UpdateStructureMesh();
		}

		ModifyStructureLength_Server(GrabbedStructure, NearestIndexToClick, GridSnappedIntersection);
	}
}

void ABGGamePlayerController::AddSplinePointToStructureSpline()
{
	if (GrabbedStructure)
	{
		if (NearestIndexToClick == -1)
		{
			FVector WorldPosition;
			FVector WorldDirection;
			DeprojectMousePositionToWorld(WorldPosition, WorldDirection);

			// Get intersection of mouse position at XY plane from GrabbedStructure's origin
			auto Intersection = FMath::LinePlaneIntersection(
				WorldPosition,
				WorldDirection * 2000.f + WorldPosition,
				GrabbedStructure->GetActorLocation(),
				FVector(0.f, 0.f, 1.f)).GridSnap(100.f);

			Intersection.Z = GrabbedStructure->GetActorLocation().Z;

			auto const ClosestKey = GrabbedStructure->GetSplineComponent()->FindInputKeyClosestToWorldLocation(
				Intersection);

			const auto TotalSplinePoints = GrabbedStructure->GetSplineComponent()->GetNumberOfSplinePoints();

			double IntegralPart;

			double const DecimalPart = modf(ClosestKey, &IntegralPart);

			// within 0.5f, plus or minus 0.15f
			if (DecimalPart >= 0.35f && DecimalPart <= 0.65f)
			{
				NearestIndexToClick = ++IntegralPart;
			}

			else if (DecimalPart < 0.35f && IntegralPart == 0)
			{
				NearestIndexToClick = IntegralPart;
			}

			else if ((DecimalPart > 0.65f || DecimalPart == 0.f) && IntegralPart == TotalSplinePoints - 1)
			{
				NearestIndexToClick = TotalSplinePoints;
			}

			if (NearestIndexToClick >= 0)
			{
				AddSplinePointToStructureSpline_Server(GrabbedStructure, Intersection, NearestIndexToClick);
			}
		}

		// Delay necessary for Spline Points to be updated/shifted so that new index is grabbed for movement, and not old one
		FTimerHandle SplinePointUpdateTimer;
		GetWorldTimerManager().SetTimer(SplinePointUpdateTimer, this, &ABGGamePlayerController::ModifyStructureLength,
		                                0.2f, false);
	}
}

void ABGGamePlayerController::MoveStructure()
{
	if (GrabbedStructure)
	{
		SetStructurePhysicsAndCollision(GrabbedStructure, false, ECollisionEnabled::Type::PhysicsOnly);
		FVector Location{};

		UE_LOG(LogTemp, Warning, TEXT("Moving Structure"))

		LastHitResult.Reset();
		LastTargetedActor = nullptr;
		if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel4), true,
		                                     LastHitResult))
		{
			if (LastHitResult.GetActor()->IsA(ABGTile::StaticClass()))
			{
				UE_LOG(LogTemp, Warning, TEXT("Cursor Hit: %s"), *LastHitResult.GetActor()->GetName())

				if (!Cast<ABGTile>(LastHitResult.GetActor())->GetStaticMeshComponent()->GetVisibleFlag())
					return;

				LastTargetedActor = LastHitResult.GetActor();

				FVector ActorOrigin{};
				FVector ActorBoxExtent{};

				LastTargetedActor->GetActorBounds(false, ActorOrigin, ActorBoxExtent, false);

				Location.X = LastTargetedActor->GetActorLocation().X;
				Location.Y = LastTargetedActor->GetActorLocation().Y;
				Location.Z = LastTargetedActor->GetActorLocation().Z + ActorOrigin.Z + ActorBoxExtent.Z;
			}
			else
			{
				return;
			}
		}

		if (!HasAuthority())
		{
			GrabbedStructure->SetActorLocation(Location);
		}
		MoveStructure_Server(GrabbedStructure, Location);
	}
}

void ABGGamePlayerController::RemoveStructureInstanceAtIndex(ABGSplineStructure* StructureToModify,
                                                             FString const& InstanceName, int const& Index)
{
	if (StructureToModify)
	{
		if (!HasAuthority())
		{
			if (StructureToModify->GetInstanceComponents().Num() > 0)
			{
				for (auto Instance : StructureToModify->GetInstanceComponents())
				{
					if (Instance)
					{
						if (Instance->GetName().Equals(InstanceName))
						{
							if (auto CastInstance = Cast<UInstancedStaticMeshComponent>(Instance))
							{
								CastInstance->RemoveInstance(Index);
								return;
							}
						}
					}
				}
			}
		}
		RemoveStructureInstanceAtIndex_Server(StructureToModify, InstanceName, Index);
	}
}

void ABGGamePlayerController::ModifyInstanceMeshAtIndex(ABGSplineStructure* StructureToModify, int const& Index,
                                                        FString const& NewInstanceName,
                                                        UStaticMesh* StaticMesh,
                                                        UMaterialInstance* MaterialInstance,
                                                        FString const& OldInstanceName)
{
	if (StructureToModify)
	{
		// Just go straight to server call
		ModifyInstanceMeshAtIndex_Server(StructureToModify, Index, NewInstanceName, StaticMesh,
		                                 MaterialInstance, OldInstanceName);
	}
}

void ABGGamePlayerController::ToggleLockStructureInPlace(ABGSplineStructure* StructureToLock, bool const bLock)
{
	if (StructureToLock)
	{
		if (!HasAuthority())
		{
			StructureToLock->ToggleLockStructureInPlace(bLock);
		}
		ToggleLockStructureInPlace_Server(StructureToLock, bLock);
	}
}

void ABGGamePlayerController::ResetStructure(ABGSplineStructure* StructureToReset) const
{
	if (StructureToReset)
	{
		if (!HasAuthority())
		{
			if (StructureToReset->GetSplineComponent()->GetNumberOfSplinePoints() > 2)
			{
				// clear all spline points except for indices 0 and 1
				for (int i{StructureToReset->GetSplineComponent()->GetNumberOfSplinePoints()}; i > 2; --i)
				{
					StructureToReset->GetSplineComponent()->RemoveSplinePoint(i - 1);
				}
			}

			// reset Spline Point 0 to actor's origin
			StructureToReset->GetSplineComponent()->SetLocationAtSplinePoint(
				0, StructureToReset->GetActorLocation(), ESplineCoordinateSpace::World, true);

			// reset Spline Point 1 to 105.f away the origin
			auto Location = StructureToReset->GetActorLocation();
			Location.X += 50.f;

			StructureToReset->GetSplineComponent()->SetLocationAtSplinePoint(
				1, Location, ESplineCoordinateSpace::World, true);

			StructureToReset->UpdateStructureMesh();
		}
	}
}

void ABGGamePlayerController::DestroyStructure(ABGSplineStructure* StructureToDestroy)
{
	if (StructureToDestroy)
	{
		if (!HasAuthority())
		{
			StructureToDestroy->Destroy();
		}
		DestroyStructure_Server(StructureToDestroy);
	}
}

void ABGGamePlayerController::HandleBoardSelection()
{
	if (GrabbedBoard)
	{
		if (!GetGameMasterPermissions())
		{
			return;
		}

		FVector WorldPosition;
		FVector WorldDirection;

		DeprojectMousePositionToWorld(WorldPosition, WorldDirection);

		auto const GridSnappedIntersection = FMath::LinePlaneIntersection(WorldPosition,
		                                                                  WorldDirection * 2000.f + WorldPosition,
		                                                                  GrabbedBoard->GetActorLocation(),
		                                                                  FVector(0.f, 0.f, 1.f)).GridSnap(105.f);

		MoveBoardToLocation(GridSnappedIntersection);
	}
}

void ABGGamePlayerController::MoveBoardToLocation(FVector const& Location)
{
	if (GrabbedBoard)
	{
		if (!HasAuthority())
		{
			GrabbedBoard->SetActorLocation(Location);
		}
		MoveBoardToLocation_Server(GrabbedBoard, Location);
	}
}

void ABGGamePlayerController::ToggleTileVisibility(ABGTile* TileToToggle)
{
	if (TileToToggle)
	{
		if (!HasAuthority())
		{
			TileToToggle->ToggleTileVisibility(TileToToggle->GetStaticMeshComponent()->GetVisibleFlag());
		}
		ToggleTileVisibility_Server(TileToToggle);
	}
}

void ABGGamePlayerController::ShrinkBoard(ABGBoard* BoardToShrink)
{
	if (BoardToShrink)
	{
		if (!HasAuthority())
		{
			BoardToShrink->ShrinkBoard(BoardToShrink->GetBoardSize().X - 1, BoardToShrink->GetBoardSize().Y - 1);
		}
		ShrinkBoard_Server(BoardToShrink);
	}
}

void ABGGamePlayerController::GrowBoard(ABGBoard* BoardToGrow)
{
	if (BoardToGrow)
	{
		if (!HasAuthority())
		{
			BoardToGrow->GrowBoard(BoardToGrow->GetBoardSize().X + 1, BoardToGrow->GetBoardSize().Y + 1);
		}
		GrowBoard_Server(BoardToGrow);
	}
}

void ABGGamePlayerController::ToggleLockStructureInPlace_Server_Implementation(ABGSplineStructure* StructureToLock,
                                                                               bool const bLock)
{
	if (StructureToLock)
	{
		ABGGameplayGameModeBase::ToggleLockStructureInPlace(StructureToLock, bLock);
	}
}

void ABGGamePlayerController::ModifyInstanceMeshAtIndex_Server_Implementation(
	ABGSplineStructure* StructureToModify, int const& Index, FString const& NewInstanceName, UStaticMesh* StaticMesh,
	UMaterialInstance* MaterialInstance, FString const& OldInstanceName)
{
	if (StructureToModify)
	{
		ABGGameplayGameModeBase::ModifyInstanceMeshAtIndex(StructureToModify, Index, NewInstanceName,
		                                                   StaticMesh,
		                                                   MaterialInstance, OldInstanceName);
	}
}

void ABGGamePlayerController::MoveStructure_Server_Implementation(ABGSplineStructure* StructureToMove,
                                                                  FVector const& Location)
{
	if (StructureToMove)
	{
		ABGGameplayGameModeBase::MoveStructure(StructureToMove, Location);
	}
}

void ABGGamePlayerController::ResetStructure_Server_Implementation(ABGSplineStructure* StructureToReset)
{
	if (StructureToReset)
	{
		ABGGameplayGameModeBase::ResetStructure(StructureToReset);
	}
}

void ABGGamePlayerController::ShrinkBoard_Server_Implementation(ABGBoard* BoardToShrink)
{
	if (BoardToShrink)
	{
		ABGGameplayGameModeBase::ShrinkBoard(BoardToShrink);
	}
}

void ABGGamePlayerController::GrowBoard_Server_Implementation(ABGBoard* BoardToGrow)
{
	if (BoardToGrow)
	{
		ABGGameplayGameModeBase::GrowBoard(BoardToGrow);
	}
}

void ABGGamePlayerController::SpawnStructureAtLocation_Server_Implementation(
	FVector const& Location, FName const& PrimaryMeshName,
	FName const& PrimaryMaterialName, FName const& SecondaryMeshName,
	FName const& SecondaryMaterialName)
{
	Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this))->SpawnStructureAtLocation(
		Location, PrimaryMeshName, PrimaryMaterialName, SecondaryMeshName, SecondaryMaterialName);
	UE_LOG(LogTemp, Warning, TEXT("Spawning Structure At Location (server)"))
}

void ABGGamePlayerController::SpawnNewBoard_Server_Implementation(int const& Zed, int const& X, int const& Y)
{
	Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this))->SpawnNewBoard(Zed, X, Y);
}

void ABGGamePlayerController::DestroyToken_Server_Implementation(ABGToken* TokenToDestroy)
{
	if (TokenToDestroy)
	{
		ABGGameplayGameModeBase::DestroyToken(TokenToDestroy);
	}
}

void ABGGamePlayerController::ToggleTokenPermissionsForPlayer_Server_Implementation(ABGPlayerState* PlayerStateToToggle,
	ABGToken* TokenToToggle)
{
	if (PlayerStateToToggle && TokenToToggle)
	{
		ABGGameplayGameModeBase::ToggleTokenPermissionsForPlayer(PlayerStateToToggle, TokenToToggle);
	}
}

void ABGGamePlayerController::RotateToken_Server_Implementation(ABGToken* TokenToRotate, FRotator const& NewRotation)
{
	if (TokenToRotate)
	{
		TokenToRotate->SetActorRotation(NewRotation);
	}
}

void ABGGamePlayerController::ResetTokenRotation_Server_Implementation(ABGToken* TokenToReset)
{
	if (TokenToReset)
	{
		TokenToReset->SetActorRotation(FRotator::ZeroRotator);
	}
}

void ABGGamePlayerController::ToggleTokenLockInPlace_Server_Implementation(ABGToken* TokenToToggle, bool bLock)
{
	if (TokenToToggle)
	{
		ABGGameplayGameModeBase::ToggleTokenLockInPlace(TokenToToggle, bLock);
	}
}

void ABGGamePlayerController::SpawnTokenAtLocation_Server_Implementation(
	FVector const& Location, FName const& MeshName, FName const& MaterialName)
{
	Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this))->SpawnTokenAtLocation(
		Location, MeshName, MaterialName);

	UE_LOG(LogTemp, Warning, TEXT("Spawning Token At Location (server)"))
}

void ABGGamePlayerController::MoveTokenToLocation_Server_Implementation(ABGToken* TokenToMove, FVector const& Location,
                                                                        FRotator const TokenRotation)
{
	if (TokenToMove)
	{
		ABGGameplayGameModeBase::MoveTokenToLocation(TokenToMove, Location, TokenRotation);
	}
}

void ABGGamePlayerController::SetTokenCollisionAndPhysics_Server_Implementation(ABGToken* TokenToModify,
	bool const bPhysicsOn, bool const bGravityOn,
	ECollisionEnabled::Type const CollisionType)
{
	if (TokenToModify)
	{
		ABGGameplayGameModeBase::SetTokenPhysicsAndCollision(TokenToModify, bPhysicsOn, bGravityOn, CollisionType);
	}
}

void ABGGamePlayerController::ModifyStructureLength_Server_Implementation(
	ABGSplineStructure* StructureToModify, int const& PointIndex,
	FVector const& NewLocation)
{
	if (StructureToModify)
	{
		ABGGameplayGameModeBase::ModifyStructureLength(StructureToModify, PointIndex, NewLocation);
	}
}

void ABGGamePlayerController::AddSplinePointToStructureSpline_Server_Implementation(
	ABGSplineStructure* StructureToModify,
	FVector const& ClickLocation, int const& Index)
{
	if (StructureToModify)
	{
		ABGGameplayGameModeBase::AddSplinePointToStructureSpline(StructureToModify, ClickLocation,
		                                                         Index);
	}
}

void ABGGamePlayerController::RotateToken(float Value)
{
	if (Value != 0)
		if (GrabbedToken)
		{
			auto const OriginalRotation = GrabbedToken->GetActorRotation().GetDenormalized();
			float const Remainder = FMath::Fmod(OriginalRotation.Yaw, 45.f);

			// If we have a Yaw that is greater than or equal to 360 degrees, use 0 instead
			int Quotient = (OriginalRotation.Yaw > 337.5f ? 0 : OriginalRotation.Yaw) / 45;

			UE_LOG(LogTemp, Warning, TEXT("Incoming Yaw: %f"), OriginalRotation.Yaw)
			UE_LOG(LogTemp, Warning, TEXT("Remainder: %f"), Remainder)
			UE_LOG(LogTemp, Warning, TEXT("Quotient: %d"), Quotient)

			// if our Yaw is close to 360 then don't upgrade the Quotient (lest we shoot past 45 and go to 90)
			if (Remainder >= 22.5f && OriginalRotation.Yaw < 337.5f)
				++Quotient;

			Quotient *= 45;

			if (Value > 0)
				Quotient += 45;
			else if (Value < 0)
			{
				Quotient -= 45;

				Quotient < 0 ? Quotient += 360 : Quotient;
			}

			UE_LOG(LogTemp, Warning, TEXT("New Angle: %d"), Quotient)

			auto NewRotation = FRotator(0.f, Quotient, 0.f);
			NewRotation.Normalize();

			if (!HasAuthority())
			{
				GrabbedToken->SetActorRotation(NewRotation);
			}
			RotateToken_Server(GrabbedToken, NewRotation);
		}
}

void ABGGamePlayerController::ResetTokenRotation(ABGToken* TokenToReset)
{
	if (TokenToReset)
	{
		if (!HasAuthority())
		{
			TokenToReset->SetActorRotation(FRotator::ZeroRotator);
		}
		ResetTokenRotation_Server(TokenToReset);
	}
}

void ABGGamePlayerController::ToggleTokenLockInPlace(ABGToken* TokenToToggle, bool bLock)
{
	if (TokenToToggle)
	{
		if (!HasAuthority())
		{
			TokenToToggle->ToggleLockTokenInPlace(bLock);
		}
		ToggleTokenLockInPlace_Server(TokenToToggle, bLock);
	}
}

void ABGGamePlayerController::ToggleTokenPermissionsForPlayer(ABGPlayerState* PlayerStateToToggle,
                                                              ABGToken* TokenToToggle)
{
	if (PlayerStateToToggle && TokenToToggle)
	{
		if (!HasAuthority())
		{
			TokenToToggle->PlayerHasPermissions(PlayerStateToToggle)
				? TokenToToggle->RemovePlayerFromPermissionsArray(PlayerStateToToggle)
				: TokenToToggle->AddPlayerToPermissionsArray(PlayerStateToToggle);
		}

		ToggleTokenPermissionsForPlayer_Server(PlayerStateToToggle, TokenToToggle);
	}
}

void ABGGamePlayerController::DestroyToken(ABGToken* TokenToDestroy)
{
	if (TokenToDestroy)
	{
		DestroyToken_Server(TokenToDestroy);
	}
}

void ABGGamePlayerController::MoveBoardToLocation_Server_Implementation(ABGBoard* BoardToMove,
                                                                        FVector const& NewLocation)
{
	if (BoardToMove)
	{
		BoardToMove->SetActorLocation(NewLocation);
	}
}

void ABGGamePlayerController::ToggleTileVisibility_Server_Implementation(ABGTile* TileToToggle)
{
	if (TileToToggle)
	{
		ABGGameplayGameModeBase::ToggleTileVisibility(TileToToggle);
	}
}

bool ABGGamePlayerController::GetGameMasterPermissions() const
{
	if (auto const PS = GetPlayerState<ABGPlayerState>())
		if (PS->GetPlayerInfo().bGameMasterPermissions)
			return true;
	return false;
}

void ABGGamePlayerController::UpdateTransformOnServer_Implementation(FTransform NewTransform)
{
	GetPawn()->SetActorTransform(NewTransform);
}

void ABGGamePlayerController::SetStructurePhysicsAndCollision_Server_Implementation(
	ABGSplineStructure* StructureToModify,
	bool const bGravityOn,
	ECollisionEnabled::Type const CollisionType)
{
	if (StructureToModify)
	{
		ABGGameplayGameModeBase::SetStructurePhysicsAndCollision(StructureToModify, bGravityOn,
		                                                         CollisionType);
	}
}

void ABGGamePlayerController::DestroyStructure_Server_Implementation(ABGSplineStructure* StructureToDestroy)
{
	if (StructureToDestroy)
	{
		ABGGameplayGameModeBase::DestroyStructure(StructureToDestroy);
	}
}

void ABGGamePlayerController::RemoveStructureInstanceAtIndex_Server_Implementation(
	ABGSplineStructure* StructureToModify, FString const& InstanceName, int const& Index)
{
	if (StructureToModify)
	{
		ABGGameplayGameModeBase::RemoveStructureInstanceAtIndex(StructureToModify, InstanceName, Index);
	}
}
