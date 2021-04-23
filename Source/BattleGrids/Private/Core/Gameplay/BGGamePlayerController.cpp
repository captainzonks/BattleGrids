// © 2021 Matthew Barham. All Rights Reserved.

#include "Core/Gameplay/BGGamePlayerController.h"

#include "Actors/BGBoard.h"
#include "Actors/BGSplineStructure.h"
#include "Actors/BGTile.h"
#include "Characters/BGToken.h"
#include "Actors/Structures/BGDoor.h"
#include "Components/SplineComponent.h"
#include "Core/BGGameInstance.h"
#include "Core/BGPlayerState.h"
#include "Core/Gameplay/BGGameplayGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UI/BGInGamePlayerList.h"
#include "UI/BGLobbyMenu.h"

ABGGamePlayerController::ABGGamePlayerController()
{
	bReplicates = true;
}

void ABGGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetupGameUI();
}

void ABGGamePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	OutlineObject();
}

void ABGGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Handle Select (Default: Left Click w/ Mouse)
	InputComponent->BindAction("Select", IE_Pressed, this, &ABGGamePlayerController::SelectObject);
	InputComponent->BindAction("Context", IE_Pressed, this, &ABGGamePlayerController::ToggleContextMenu);

	// Token Movement Handling
	InputComponent->BindAxis("RotateToken", this, &ABGGamePlayerController::RotateToken);

	// In Game UI
	InputComponent->BindAction("PlayerList", IE_Pressed, this, &ABGGamePlayerController::ShowInGamePlayerListMenu);
}

void ABGGamePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME(ABGGamePlayerController, ControlMode)
	// DOREPLIFETIME(ABGGamePlayerController, GrabbedObject)
	DOREPLIFETIME(ABGGamePlayerController, TokenNames)
	DOREPLIFETIME(ABGGamePlayerController, StructureNames)
}

void ABGGamePlayerController::SetupGameUI_Implementation()
{
	auto GameInstance = GetGameInstance<UBGGameInstance>();
	if (GameInstance)
	{
		if (GameInstance->GetLobby())
		{
			GameInstance->GetLobby()->Teardown();
		}
		GameInstance->LoadGameHUDWidget();
		GameInstance->ToggleLoadingScreen(false);
	}
}

void ABGGamePlayerController::UpdateTransformOnServer_Implementation(FTransform const& NewTransform)
{
	auto const MyPawn = GetPawn();
	if (MyPawn)
	{
		GetPawn()->SetActorTransform(NewTransform);
	}
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

void ABGGamePlayerController::LoadObjectType()
{
	if (LastHitResult.bBlockingHit && LastHitResult.GetActor()->IsValidLowLevel())
	{
		LastClickedActor = LastHitResult.GetActor();

		if ((GrabbedToken = Cast<ABGToken>(LastClickedActor))->IsValidLowLevel())
		{
			UE_LOG(LogTemp, Warning, TEXT("LastClickedActor: %s"), *LastClickedActor->GetName())

			GrabbedObject = EBGObjectType::Token;
			return;
		}

		if ((GrabbedStructure = Cast<ABGSplineStructure>(LastClickedActor))->IsValidLowLevel())
		{
			UE_LOG(LogTemp, Warning, TEXT("LastClickedActor: %s"), *LastClickedActor->GetName())

			GrabbedObject = EBGObjectType::Structure;
				
			/** Add a new spline point if LeftAlt was held down */
			if (GetInputAnalogKeyState(EKeys::LeftAlt) == 1)
			{
				AddSplinePointToSplineStructure();
			}

			/** If we haven't stored a NearestIndexToClick yet, get one */
			if (NearestIndexToClick < 0)
			{
				if (GrabbedStructure->GetInstancedStaticMeshComponentByTag("WallInstance")->GetInstanceCount() < 2)
				{
					NearestIndexToClick = 1;
				}
				else
				{
					FVector GridSnappedIntersection;
					NearestIndexToClick = FMath::RoundToInt(
						GetClosestKeyOnSplineAtMousePosition(
							GrabbedStructure, GridSnappedIntersection));
				}
			}

			return;
		}

		if ((GrabbedBoard = Cast<ABGBoard>(LastClickedActor))->IsValidLowLevel())
		{
			UE_LOG(LogTemp, Warning, TEXT("LastClickedActor: %s"), *LastClickedActor->GetName())

			GrabbedObject = EBGObjectType::Board;
		}
	}
}

void ABGGamePlayerController::SelectObject()
{
	/** If a context menu is open, close it */
	if (LastHitResult.GetActor() && LastHitResult.GetActor()->Implements<UBGActorInterface>())
	{
		Cast<IBGActorInterface>(LastHitResult.GetActor())->CloseContextMenu();
	}

	/**
	 * Refresh LastHitResult with a new trace under cursor
	 * If we haven't loaded an object yet, search on the Grab trace channel
	 * If we have loaded an object, execute on the Tile trace channel
	 */
	GetHitResultUnderCursorByChannel(
		UEngineTypes::ConvertToTraceType(GrabbedObject == EBGObjectType::None
			                                 ? ECC_GRAB
			                                 : ECC_TILE), true, LastHitResult);

	switch (GrabbedObject)
	{
		/** Load an object into our LeftClick if we don't have one yet */
	case EBGObjectType::None:
		LoadObjectType();
		break;

		/** Handle executing specific functions if we already have an object loaded into our Left Click */
	case EBGObjectType::Token:
		if (LastHitResult.bBlockingHit && LastHitResult.GetActor()->IsValidLowLevel()
			&& LastHitResult.GetActor()->IsA(ABGTile::StaticClass()))
		{
			LastClickedActor = LastHitResult.GetActor();
			// TODO: remove the Holding parameter, it's unused
			MoveTokenToLocation(false);
			ReleaseObject();
		}
		break;
	case EBGObjectType::Structure:
		if (LastHitResult.bBlockingHit && LastHitResult.GetActor()->IsValidLowLevel()
			&& LastHitResult.GetActor()->IsA(ABGTile::StaticClass()))
		{
			LastClickedActor = LastHitResult.GetActor();
			HandleSplineStructureSelection();
			ReleaseObject();
		}
		break;
	case EBGObjectType::Board:
		HandleBoardSelection();
		break;
	default: ;
	}
}

void ABGGamePlayerController::ReleaseObject()
{
	GrabbedObject = EBGObjectType::None;
	GrabbedToken = nullptr;
	GrabbedStructure = nullptr;
	GrabbedBoard = nullptr;
	LastClickedActor = nullptr;
	LastHitResult.Reset();
	NearestIndexToClick = -1;
}

void ABGGamePlayerController::ToggleContextMenu_Implementation()
{
	if (LastHitResult.GetActor() && LastHitResult.GetActor()->Implements<UBGActorInterface>())
	{
		Cast<IBGActorInterface>(LastHitResult.GetActor())->CloseContextMenu();
	}

	ReleaseObject();

	if (GetHitResultUnderCursorByChannel(
		UEngineTypes::ConvertToTraceType(ECC_RIGHT_CLICK), true, LastHitResult))
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *LastHitResult.GetActor()->GetName())

		if (LastHitResult.GetActor() && LastHitResult.GetActor()->Implements<UBGActorInterface>())
		{
			auto CastInterface = Cast<IBGActorInterface>(LastHitResult.GetActor());
			if (CastInterface)
			{
				auto ContextMenu = CastInterface->GetContextMenu();
				if (!ContextMenu)
				{
					UE_LOG(LogTemp, Warning, TEXT("No Context Menu"))
					return;
				}
				ContextMenu->SetHitResult(LastHitResult);
				ContextMenu->Update();
				CastInterface->ToggleContextMenu();

				if (CastInterface->GetContextMenu()->GetIsVisible())
				{
					FInputModeUIOnly const InputMode;
					SetInputMode(InputMode);
				}
				else
				{
					FInputModeGameAndUI const InputMode;
					SetInputMode(InputMode);
				}
				return;
			}
			UE_LOG(LogTemp, Warning, TEXT("No Actor Interface for Context Menu Retrieval"))
		}
	}
}

void ABGGamePlayerController::MoveTokenToLocation(bool const bHolding)
{
	if (GrabbedToken && LastClickedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveTokenToLocation(): LastClickedActor = %s"), *LastClickedActor->GetName())

		FRotator const Rotation = FRotator(0.f, GrabbedToken->GetActorRotation().Yaw, 0.f);

		float ZedValue;
		bHolding ? ZedValue = 100.f : ZedValue = 50.f;

		FVector Location;

		if (Cast<ABGSplineStructure>(LastClickedActor))
		{
			// Don't move tokens on Bases
			if (LastHitResult.GetComponent()->GetName().Contains("Base"))
			{
				return;
			}

			Location = LastHitResult.GetComponent()->Bounds.Origin + FVector(
				0.f, 0.f, LastHitResult.GetComponent()->Bounds.BoxExtent.Z + ZedValue);
		}
		else
		{
			FVector ActorOrigin{};
			FVector ActorBoxExtent{};

			LastClickedActor->GetActorBounds(false, ActorOrigin, ActorBoxExtent, false);

			Location.X = ActorOrigin.X;
			Location.Y = ActorOrigin.Y;
			Location.Z = ZedValue + ActorOrigin.Z + ActorBoxExtent.Z;
		}

		MoveTokenToLocation_Server(GrabbedToken, Location, Rotation);
	}
}

void ABGGamePlayerController::HandleSplineStructureSelection()
{
	// TODO: handle game master permissions
	if (GrabbedStructure->GetLocked())
		return;

	switch (ControlMode)
	{
	case EBGControlMode::Build:
		ModifySplineStructureLength();
		break;
	case EBGControlMode::Edit: break;
	case EBGControlMode::Move:
		MoveSplineStructure();
		break;
	default: ;
	}
}

void ABGGamePlayerController::SpawnSplineStructureAtLocation(FVector const& Location, FName const& WallStaticMeshName,
                                                             FName const& WallMaskedMaterialInstanceName,
                                                             FName const& CornerStaticMeshName,
                                                             FName const& CornerMaskedMaterialInstanceName,
                                                             FName const& BaseStaticMeshName,
                                                             FName const& BaseMaterialInstanceName)
{
	SpawnSplineStructureAtLocation_Server(Location, WallStaticMeshName, WallMaskedMaterialInstanceName,
	                                      CornerStaticMeshName, CornerMaskedMaterialInstanceName, BaseStaticMeshName,
	                                      BaseMaterialInstanceName);
}

void ABGGamePlayerController::ModifySplineStructureLength()
{
	if (GrabbedStructure && LastHitResult.GetActor()->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cursor Hit: %s"), *LastHitResult.GetActor()->GetName())

		/** Check if the tile is visible */
		if (LastHitResult.GetActor()->IsA(ABGTile::StaticClass())
			&& !Cast<ABGTile>(LastHitResult.GetActor())->GetStaticMeshComponent()->GetVisibleFlag())
		{
			return;
		}

		/** Snap the Target Location to grid, and match the Z axis to the current SplineStructure Z value */
		auto GridSnappedIntersection = LastHitResult.GetActor()->GetActorLocation().GridSnap(100.f);
		GridSnappedIntersection.Z = GrabbedStructure->GetActorLocation().Z;

		ModifyStructureLength_Server(GrabbedStructure, NearestIndexToClick, GridSnappedIntersection);
	}
}

float ABGGamePlayerController::GetClosestKeyOnSplineAtMousePosition(ABGSplineStructure* SplineStructure, FVector& OutIntersection) const
{
	FVector WorldPosition;
	FVector WorldDirection;
	DeprojectMousePositionToWorld(WorldPosition, WorldDirection);

	OutIntersection = FMath::LinePlaneIntersection(
		WorldPosition,
		WorldDirection * 2000.f + WorldPosition,
		SplineStructure->GetActorLocation(),
		FVector(0.f, 0.f, 1.f)).GridSnap(100.f);

	/** On original Z-level */
	OutIntersection.Z = SplineStructure->GetActorLocation().Z;

	return SplineStructure->GetSplineComponent()->FindInputKeyClosestToWorldLocation(OutIntersection);
}

void ABGGamePlayerController::AddSplinePointToSplineStructure()
{
	if (GrabbedStructure)
	{
		if (NearestIndexToClick < 0)
		{
			FVector Intersection;
			auto const ClosestKey = GetClosestKeyOnSplineAtMousePosition(GrabbedStructure, Intersection);

			const auto TotalSplinePoints = GrabbedStructure->GetSplineComponent()->GetNumberOfSplinePoints();

			/** Split apart the Integral and Decimal of the ClosestKey */
			double IntegralPart {};
			double const DecimalPart = modf(ClosestKey, &IntegralPart);

			/**
			* Situation of between 0 and 1 spline indices:
			* if we clicked close to 0, set the nearest index to 0
			* to create a new 0 spline point
			*/
			if (DecimalPart < 0.35f && IntegralPart == 0)
			{
				NearestIndexToClick = IntegralPart;
			}

			/**
			 * If our click was inside a wall section,
			 * we want to get one greater than our integral
			 * in order to shunt the last index up and put a new one
			 * in the middle of the wall section
			 */
			else if (DecimalPart <= 0.85f)
			{
				NearestIndexToClick = ++IntegralPart;
			}

			/**
			 * If we magically managed to click on the very end of the spline
			 * and get the greatest spline index possible
			 * OR
			 * We click anywhere near the end of a wall section that is the greatest
			 * spline index possible (2 less than total points)
			 */
			else if (DecimalPart == 0.f && IntegralPart == TotalSplinePoints - 1
				|| DecimalPart > 0.85f && IntegralPart == TotalSplinePoints - 2)
			{
				NearestIndexToClick = TotalSplinePoints;
			}

			if (NearestIndexToClick >= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("NearestIndexToClick: %i"), NearestIndexToClick)
				AddSplinePointToSplineStructure_Server(GrabbedStructure, Intersection, NearestIndexToClick);
			}
		}
	}
}

void ABGGamePlayerController::MoveSplineStructure()
{
	if (GrabbedStructure)
	{
		FVector Location{};

		UE_LOG(LogTemp, Warning, TEXT("Moving Structure"))

		LastHitResult.Reset();
		LastClickedActor = nullptr;
		if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel4), true,
		                                     LastHitResult))
		{
			if (LastHitResult.GetActor()->IsA(ABGTile::StaticClass()))
			{
				UE_LOG(LogTemp, Warning, TEXT("Cursor Hit: %s"), *LastHitResult.GetActor()->GetName())

				if (!Cast<ABGTile>(LastHitResult.GetActor())->GetStaticMeshComponent()->GetVisibleFlag())
					return;

				LastClickedActor = LastHitResult.GetActor();

				FVector ActorOrigin{};
				FVector ActorBoxExtent{};

				LastClickedActor->GetActorBounds(false, ActorOrigin, ActorBoxExtent, false);

				Location.X = LastClickedActor->GetActorLocation().X;
				Location.Y = LastClickedActor->GetActorLocation().Y;
				Location.Z = LastClickedActor->GetActorLocation().Z + ActorOrigin.Z + ActorBoxExtent.Z;
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
		MoveSplineStructure_Server(GrabbedStructure, Location);
	}
}

void ABGGamePlayerController::ModifySplineStructureInstanceMeshAtIndex(ABGSplineStructure* StructureToModify,
                                                                       int const& Index,
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

void ABGGamePlayerController::SpawnStructureActorAtSplineStructureIndex(ABGSplineStructure* StructureToModify,
                                                                        int const& Index,
                                                                        TSubclassOf<ABGStructure> StructureClassToSpawn,
                                                                        FString const& OldInstanceName)
{
	if (StructureToModify && StructureClassToSpawn)
	{
		SpawnStructureActorAtSplineStructureIndex_Server(StructureToModify, Index, StructureClassToSpawn,
		                                                 OldInstanceName);
	}
}

void ABGGamePlayerController::RemoveInstanceAtIndexOnSplineStructure(ABGSplineStructure* StructureToModify,
                                                                     int const& Index, FString const& InstanceName)
{
	if (StructureToModify)
	{
		RemoveInstanceAtIndexOnSplineStructure_Server(StructureToModify, Index, InstanceName);
	}
}

void ABGGamePlayerController::RestoreInstanceAtIndexOnSplineStructure(ABGSplineStructure* StructureToModify,
                                                                      int const& Index,
                                                                      FTransform const& NewInstanceTransform,
                                                                      FString const& InstanceName)
{
	if (StructureToModify)
	{
		RestoreInstanceAtIndexOnSplineStructure_Server(StructureToModify, Index, NewInstanceTransform, InstanceName);
	}
}

void ABGGamePlayerController::DestroyStructureActor(ABGStructure* StructureToRemove)
{
	if (StructureToRemove)
	{
		DestroyStructureActor_Server(StructureToRemove);
	}
}

void ABGGamePlayerController::ToggleLockSplineStructureInPlace(ABGSplineStructure* SplineStructureToLock,
                                                               bool const bNewLocked)
{
	if (SplineStructureToLock)
	{
		ToggleLockSplineStructureInPlace_Server(SplineStructureToLock, bNewLocked);
	}
}

void ABGGamePlayerController::ToggleLockStructure(ABGStructure* StructureToLock, bool const bNewLocked)
{
	if (StructureToLock)
	{
		ToggleLockStructure_Server(StructureToLock, bNewLocked);
	}
}

void ABGGamePlayerController::ToggleDoorOpenClose(ABGDoor* DoorToToggle)
{
	if (DoorToToggle)
	{
		ToggleDoorOpenClose_Server(DoorToToggle);
	}
}

void ABGGamePlayerController::ResetSplineStructure(ABGSplineStructure* SplineStructureToReset) const
{
	if (SplineStructureToReset)
	{
		if (!HasAuthority())
		{
			if (SplineStructureToReset->GetSplineComponent()->GetNumberOfSplinePoints() > 2)
			{
				// clear all spline points except for indices 0 and 1
				for (int i{SplineStructureToReset->GetSplineComponent()->GetNumberOfSplinePoints()}; i > 2; --i)
				{
					SplineStructureToReset->GetSplineComponent()->RemoveSplinePoint(i - 1);
				}
			}

			// reset Spline Point 0 to actor's origin
			SplineStructureToReset->GetSplineComponent()->SetLocationAtSplinePoint(
				0, SplineStructureToReset->GetActorLocation(), ESplineCoordinateSpace::World, true);

			// reset Spline Point 1 to 105.f away the origin
			auto Location = SplineStructureToReset->GetActorLocation();
			Location.X += 50.f;

			SplineStructureToReset->GetSplineComponent()->SetLocationAtSplinePoint(
				1, Location, ESplineCoordinateSpace::World, true);

			SplineStructureToReset->UpdateSplineStructureMesh();
		}
	}
}

void ABGGamePlayerController::DestroySplineStructure(ABGSplineStructure* SplineStructureToDestroy)
{
	if (SplineStructureToDestroy)
	{
		if (!HasAuthority())
		{
			SplineStructureToDestroy->Destroy();
		}
		DestroySplineStructure_Server(SplineStructureToDestroy);
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

void ABGGamePlayerController::ShowInGamePlayerListMenu()
{
	auto const GameInstance = GetGameInstance<UBGGameInstance>();
	if (!ensure(GameInstance)) return;

	auto InGamePlayList = GameInstance->GetInGamePlayerList();
	if (!InGamePlayList)
	{
		GameInstance->LoadInGamePlayerListWidget();
	}
	if (InGamePlayList && InGamePlayList->IsInViewport())
	{
		InGamePlayList->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		InGamePlayList->SetVisibility(ESlateVisibility::Visible);
	}
}

void ABGGamePlayerController::ToggleDoorOpenClose_Server_Implementation(ABGDoor* DoorToToggle)
{
	if (HasAuthority() && DoorToToggle)
	{
		DoorToToggle->ToggleOpenClose();
	}
}

void ABGGamePlayerController::ToggleLockStructure_Server_Implementation(
	ABGStructure* StructureToLock, bool const bNewLocked)
{
	if (HasAuthority() && StructureToLock)
	{
		StructureToLock->ToggleStructureLock(bNewLocked);
	}
}

void ABGGamePlayerController::RestoreInstanceAtIndexOnSplineStructure_Server_Implementation(
	ABGSplineStructure* StructureToModify, int const& Index, FTransform const& NewInstanceTransform,
	FString const& InstanceName)
{
	if (HasAuthority() && StructureToModify)
	{
		StructureToModify->RestoreInstanceMeshAtIndex(Index, NewInstanceTransform, InstanceName);
	}
}

void ABGGamePlayerController::RemoveInstanceAtIndexOnSplineStructure_Server_Implementation(
	ABGSplineStructure* StructureToModify, int const& Index, FString const& InstanceName)
{
	if (HasAuthority() && StructureToModify)
	{
		StructureToModify->RemoveInstanceMeshAtIndex(Index, InstanceName);
	}
}

void ABGGamePlayerController::DestroyStructureActor_Server_Implementation(
	ABGStructure* StructureToRemove)
{
	if (HasAuthority() && StructureToRemove)
	{
		ABGGameplayGameModeBase::DestroyStructureActor(StructureToRemove);
	}
}

void ABGGamePlayerController::SpawnStructureActorAtSplineStructureIndex_Server_Implementation(
	ABGSplineStructure* StructureToModify, int const& Index, TSubclassOf<ABGStructure> StructureClassToSpawn,
	FString const& OldInstanceName)
{
	if (HasAuthority() && StructureToModify && StructureClassToSpawn)
	{
		ABGGameplayGameModeBase::SpawnStructureActorAtSplineStructureIndex(
			StructureToModify, Index, StructureClassToSpawn, OldInstanceName);
	}
}

void ABGGamePlayerController::ToggleLockSplineStructureInPlace_Server_Implementation(
	ABGSplineStructure* SplineStructureToLock,
	bool const bNewLocked)
{
	if (HasAuthority() && SplineStructureToLock)
	{
		SplineStructureToLock->ToggleLockStructureInPlace(bNewLocked);
	}
}

void ABGGamePlayerController::ModifyInstanceMeshAtIndex_Server_Implementation(
	ABGSplineStructure* StructureToModify, int const& Index, FString const& NewInstanceName, UStaticMesh* StaticMesh,
	UMaterialInstance* MaterialInstance, FString const& OldInstanceName)
{
	if (HasAuthority() && StructureToModify)
	{
		ABGGameplayGameModeBase::ModifySplineStructureInstanceMeshAtIndex(StructureToModify, Index, NewInstanceName,
		                                                                  StaticMesh,
		                                                                  MaterialInstance, OldInstanceName);
	}
}

void ABGGamePlayerController::MoveSplineStructure_Server_Implementation(ABGSplineStructure* StructureToMove,
                                                                        FVector const& Location)
{
	if (HasAuthority() && StructureToMove)
	{
		ABGGameplayGameModeBase::MoveSplineStructure(StructureToMove, Location);
	}
}

void ABGGamePlayerController::ResetSplineStructure_Server_Implementation(ABGSplineStructure* StructureToReset)
{
	if (HasAuthority() && StructureToReset)
	{
		ABGGameplayGameModeBase::ResetSplineStructure(StructureToReset);
	}
}

void ABGGamePlayerController::ShrinkBoard_Server_Implementation(ABGBoard* BoardToShrink)
{
	if (HasAuthority() && BoardToShrink)
	{
		ABGGameplayGameModeBase::ShrinkBoard(BoardToShrink);
	}
}

void ABGGamePlayerController::GrowBoard_Server_Implementation(ABGBoard* BoardToGrow)
{
	if (HasAuthority() && BoardToGrow)
	{
		ABGGameplayGameModeBase::GrowBoard(BoardToGrow);
	}
}

void ABGGamePlayerController::SpawnSplineStructureAtLocation_Server_Implementation(
	FVector const& Location, FName const& WallStaticMeshName,
	FName const& WallMaskedMaterialInstanceName,
	FName const& CornerStaticMeshName,
	FName const& CornerMaskedMaterialInstanceName,
	FName const& BaseStaticMeshName,
	FName const& BaseMaterialInstanceName)
{
	if (HasAuthority())
	{
		Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this))->SpawnSplineStructureAtLocation(
			Location, WallStaticMeshName, WallMaskedMaterialInstanceName, CornerStaticMeshName,
			CornerMaskedMaterialInstanceName, BaseStaticMeshName,
			BaseMaterialInstanceName);
		UE_LOG(LogTemp, Warning, TEXT("Spawning Structure At Location (server)"))
	}
}

void ABGGamePlayerController::SpawnNewBoard_Server_Implementation(int const& Zed, int const& X, int const& Y)
{
	if (HasAuthority())
	{
		Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this))->SpawnNewBoard(Zed, X, Y);
	}
}

void ABGGamePlayerController::DestroyToken_Server_Implementation(ABGToken* TokenToDestroy)
{
	if (HasAuthority() && TokenToDestroy)
	{
		ABGGameplayGameModeBase::DestroyToken(TokenToDestroy);
	}
}

void ABGGamePlayerController::ToggleTokenPermissionsForPlayer_Server_Implementation(ABGPlayerState* PlayerStateToToggle,
	ABGToken* TokenToToggle)
{
	if (HasAuthority() && PlayerStateToToggle && TokenToToggle)
	{
		ABGGameplayGameModeBase::ToggleTokenPermissionsForPlayer(PlayerStateToToggle, TokenToToggle);
	}
}

void ABGGamePlayerController::RotateToken_Server_Implementation(ABGToken* TokenToRotate, FRotator const& NewRotation)
{
	if (HasAuthority() && TokenToRotate)
	{
		TokenToRotate->SetActorRotation(NewRotation);
	}
}

void ABGGamePlayerController::ResetTokenRotation_Server_Implementation(ABGToken* TokenToReset)
{
	if (HasAuthority() && TokenToReset)
	{
		TokenToReset->SetActorRotation(FRotator::ZeroRotator);
	}
}

void ABGGamePlayerController::ToggleTokenLockInPlace_Server_Implementation(ABGToken* TokenToToggle, bool bLock)
{
	if (HasAuthority() && TokenToToggle)
	{
		ABGGameplayGameModeBase::ToggleTokenLockInPlace(TokenToToggle, bLock);
	}
}

void ABGGamePlayerController::SpawnTokenAtLocation_Server_Implementation(
	FVector const& Location, FName const& MeshName, FName const& MaterialName)
{
	if (HasAuthority())
	{
		Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this))->SpawnTokenAtLocation(
			Location, MeshName, MaterialName);

		UE_LOG(LogTemp, Warning, TEXT("Spawning Token At Location (server)"))
	}
}

void ABGGamePlayerController::MoveTokenToLocation_Server_Implementation(ABGToken* TokenToMove, FVector const& Location,
                                                                        FRotator const TokenRotation)
{
	if (HasAuthority() && TokenToMove)
	{
		UE_LOG(LogTemp, Warning, TEXT("Moving Token To Location (server)"))
		ABGGameplayGameModeBase::MoveTokenToLocation(TokenToMove, Location, TokenRotation);
	}
}

void ABGGamePlayerController::ModifyStructureLength_Server_Implementation(
	ABGSplineStructure* StructureToModify, int const& PointIndex,
	FVector const& NewLocation)
{
	if (StructureToModify)
	{
		ABGGameplayGameModeBase::ModifySplineStructureLength(StructureToModify, PointIndex, NewLocation);
	}
}

void ABGGamePlayerController::AddSplinePointToSplineStructure_Server_Implementation(
	ABGSplineStructure* StructureToModify,
	FVector const& ClickLocation, int const& Index)
{
	if (HasAuthority() && StructureToModify)
	{
		ABGGameplayGameModeBase::AddSplinePointToSplineStructure(StructureToModify, ClickLocation,
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
	if (auto const BGPlayerState = GetPlayerState<ABGPlayerState>())
		if (BGPlayerState->GetPlayerInfo().bGameMasterPermissions)
			return true;
	return false;
}

void ABGGamePlayerController::OutlineObject()
{
	FHitResult HitResult;
	if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel6), true, HitResult))
	{
		if (HitResult.GetActor()->IsValidLowLevel())
		{
			TArray<UStaticMeshComponent*> StaticMeshComponents;

			// Do we have a previous CurrentOutlinedActor, and is it not equal to the newly hit Actor?
			if (CurrentOutlinedActor && CurrentOutlinedActor
				!=
				HitResult.GetActor()
			)
			{
				// Turn off the RenderCustomDepth on all Static Mesh Components of the previous CurrentOutlinedActor
				CurrentOutlinedActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

				if (StaticMeshComponents.Num() > 0)
				{
					for (auto Comp : StaticMeshComponents)
					{
						// UE_LOG(LogTemp, Warning, TEXT("Setting Render Custom Depth : False (1)"))
						Comp->SetRenderCustomDepth(false);
					}
				}
			}

			// Clear the array
			StaticMeshComponents.Empty();

			// set the pointer to the newly hit Actor
			CurrentOutlinedActor = HitResult.GetActor();

			// Fill it again with the new hit Actor's Static Mesh Components
			HitResult.GetActor()->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

			// Turn the RenderCustomDepth on for the new hit Actor
			if (StaticMeshComponents.Num() > 0)
			{
				for (auto Comp : StaticMeshComponents)
				{
					// UE_LOG(LogTemp, Warning, TEXT("Setting Render Custom Depth : True"))
					Comp->SetRenderCustomDepth(true);
				}
			}
		}
	}
	else
	{
		// Do we have a previous CurrentOutlinedActor
		if (CurrentOutlinedActor)
		{
			// Turn off the RenderCustomDepth on all Static Mesh Components of the previous CurrentOutlinedActor
			TArray<UStaticMeshComponent*> StaticMeshComponents;
			CurrentOutlinedActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);

			if (StaticMeshComponents.Num() > 0)
			{
				for (auto Comp : StaticMeshComponents)
				{
					// UE_LOG(LogTemp, Warning, TEXT("Setting Render Custom Depth : False (2)"))
					Comp->SetRenderCustomDepth(false);
				}
			}
		}
	}
}

void ABGGamePlayerController::DestroySplineStructure_Server_Implementation(ABGSplineStructure* StructureToDestroy)
{
	if (StructureToDestroy)
	{
		ABGGameplayGameModeBase::DestroySplineStructure(StructureToDestroy);
	}
}
