// © 2021 Matthew Barham. All Rights Reserved.

#include "Core/Gameplay/BGGamePlayerController.h"

#include "Actors/Structures/BGDoor.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Characters/BGCharacter.h"
#include "Components/BGBoardComponent.h"
#include "Components/BGSplineWallComponent.h"
#include "Components/BGTileComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/MenuAnchor.h"
#include "Components/SplineComponent.h"
#include "Core/BGGameInstance.h"
#include "Core/BGPlayerState.h"
#include "Core/Gameplay/BGGameplayGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UI/BGGameHUD.h"
#include "UI/BGInGamePlayerList.h"
#include "UI/BGLobbyMenu.h"

ABGGamePlayerController::ABGGamePlayerController()
{
	bReplicates = true;
}

void ABGGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	BGGameInstance = GetGameInstance<UBGGameInstance>();

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
	InputComponent->BindAction("Select", IE_Pressed, this, &ABGGamePlayerController::SelectActor);
	InputComponent->BindAction("Context", IE_Pressed, this, &ABGGamePlayerController::ToggleContextMenu);

	// Token Movement Handling
	InputComponent->BindAxis("RotateToken", this, &ABGGamePlayerController::RotateCharacter);

	// In Game UI
	InputComponent->BindAction("PlayerList", IE_Pressed, this, &ABGGamePlayerController::ShowInGamePlayerListMenu);
}

void ABGGamePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABGGamePlayerController, TokenNames)
	DOREPLIFETIME(ABGGamePlayerController, ActorNames)
}

void ABGGamePlayerController::SelectActor()
{
	/** If a context menu is open, close it */
	if (BGGameInstance)
	{
		BGGameInstance->GetGameHUD()->CloseContextMenu();
	}

	/**
	 * Refresh LastHitResult with a new trace under cursor
	 * If we haven't loaded an object yet, search on the Grab trace channel
	 * If we have loaded an object, execute on the Tile trace channel
	 */
	GetHitResultUnderCursorByChannel(
		UEngineTypes::ConvertToTraceType(!GrabbedActor ? ECC_GRAB : ECC_TILE), true, LastHitResult);

	if (LastHitResult.bBlockingHit)
	{
		LastClickedActor = LastHitResult.GetActor();

		/** Load an object into our LeftClick if we don't have one yet */
		if (!GrabbedActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Click 1: Loading Actor"))
			LoadGrabbedActor();
			return;
		}

		/** Execute specific functions for BGActor movement */
		auto const LastClickedBGActor = Cast<ABGActor>(LastClickedActor);
		auto const GrabbedBGActor = Cast<ABGActor>(GrabbedActor);
		if (GrabbedBGActor && LastClickedBGActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit a BGActor!"))
			auto const GrabbedBGActorType = GrabbedBGActor->GetActorType();
			
			switch (GrabbedBGActorType)
			{
			case EBGActorType::None:
				break;

			case EBGActorType::Structure:
				if (LastClickedBGActor->GetActorType() == EBGActorType::Tile)
				{
					UE_LOG(LogTemp, Warning, TEXT("Click 2: Actor Type Structure"))
					HandleSplineStructureSelection();
					ReleaseGrabbedActor();
				}
				break;
			
			case EBGActorType::Light:
				if (LastClickedBGActor->GetActorType() == EBGActorType::Tile)
				{
					UE_LOG(LogTemp, Warning, TEXT("Click 2: Actor Type Light"))
					LastClickedActor = GrabbedBGActor;
					// HandleActorSelection();
				}
				ReleaseGrabbedActor();
				break;
			
			default: ;
			}
		}

		/** Execute specific functions for BGToken movement */
		auto const GrabbedBGCharacter = Cast<ABGCharacter>(GrabbedActor);
		if (GrabbedBGCharacter && LastClickedBGActor)
		{
			if (LastClickedBGActor->GetActorType() == EBGActorType::Tile)
			{
				UE_LOG(LogTemp, Warning, TEXT("Click 2: Token Movement"))
				MoveCharacterToLocation();
				ReleaseGrabbedActor();
			}
		}
	}
}

void ABGGamePlayerController::LoadGrabbedActor()
{
	if (LastClickedActor)
	{
		auto const LastClickedBGActor = Cast<ABGActor>(LastClickedActor);

		/** Did we click on a BGActor? */
		if (LastClickedBGActor && !LastClickedBGActor->GetIsLocked())
		{
			auto const LastClickedBGActorType = LastClickedBGActor->GetActorType();
			/** Don't load a tile (for now) */
			if (LastClickedBGActorType == EBGActorType::Tile) return;

			/** Store the last clicked variables as the now grabbed variables */
			GrabbedActor = LastClickedBGActor;
			GrabbedActorType = LastClickedBGActorType;

			/** Check the type and execute specific functions as needed */
			switch (GrabbedActorType)
			{
				case EBGActorType::None: break;

				case EBGActorType::Structure:
				{
					/** Try and get a SplineComponent off the Structure actor */
					auto const SplineComponent = Cast<UBGSplineWallComponent>(
						GrabbedActor->GetComponentByClass(UBGSplineWallComponent::StaticClass()));
					if (SplineComponent)
					{
						/** Add a new spline point if LeftAlt was held down */
						if (GetInputAnalogKeyState(EKeys::LeftAlt) == 1)
						{
							AddSplinePointToSplineStructure(SplineComponent);
						}
    
						/** If we haven't stored a NearestIndexToClick yet, get one */
						if (NearestIndexToClick < 0)
						{
							if (SplineComponent->GetInstancedStaticMeshComponentByTag("WallInstance")
								->GetInstanceCount() < 2)
							{
								NearestIndexToClick = 1;
							}
							else
							{
								/** Unused variable for the out parameter */
								FVector UnusedLocation;
								NearestIndexToClick = FMath::RoundToInt(
									GetClosestKeyOnSplineAtMousePosition(
										SplineComponent, UnusedLocation));
							}
						}
					}
				} break;

				case EBGActorType::Board: break;

				case EBGActorType::Tile: break;

				case EBGActorType::Light: break;

				default: ;
			}
			return;
		}

		auto const LastClickedBGCharacter = Cast<ABGCharacter>(LastClickedActor);

		/** Did we click on a BGToken? */
		if (LastClickedBGCharacter)
		{
			/** Store the last clicked variables as the now grabbed variables */
			GrabbedActor = LastClickedBGCharacter;
		}
	}
}

void ABGGamePlayerController::ReleaseGrabbedActor()
{
	GrabbedActorType = EBGActorType::None;
	GrabbedActor = nullptr;
	LastClickedActor = nullptr;
	LastHitResult.Reset();
	NearestIndexToClick = -1;
}

void ABGGamePlayerController::DestroyGameActor_Implementation(AActor* ActorToDestroy)
{
	if (ActorToDestroy)
	{
		auto const CastGameMode = GetWorld()->GetAuthGameMode<ABGGameplayGameModeBase>();
		if (CastGameMode)
		{
			CastGameMode->DestroyGameActor(ActorToDestroy);
		}
	}
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

void ABGGamePlayerController::GetRowNamesOfObjectTypeFromGameMode(EBGClassCategory const& ClassCategory)
{
	GetRowNamesOfObjectTypeFromGameMode_Server(ClassCategory);
}

void ABGGamePlayerController::GetRowNamesOfObjectTypeFromGameMode_Server_Implementation(
	EBGClassCategory const& ClassCategory)
{
	switch (ClassCategory)
	{
	case EBGClassCategory::None: break;
	case EBGClassCategory::Actor:
		ActorNames = Cast<ABGGameplayGameModeBase>(
			UGameplayStatics::GetGameMode(this))->GetRowNamesOfType(ClassCategory);
		break;
	case EBGClassCategory::Character:
		TokenNames = Cast<ABGGameplayGameModeBase>(
			UGameplayStatics::GetGameMode(this))->GetRowNamesOfType(ClassCategory);
		break;
	default: ;
	}
}

void ABGGamePlayerController::ToggleContextMenu_Implementation()
{
	ReleaseGrabbedActor();
	
	if (BGGameInstance)
	{
		if (GetHitResultUnderCursorByChannel(
			UEngineTypes::ConvertToTraceType(ECC_RIGHT_CLICK),
			true,
			LastHitResult))
		{
			BGGameInstance->GetGameHUD()->SetContextMenuHitResult(LastHitResult);
			UWidgetLayoutLibrary::SlotAsCanvasSlot(
				BGGameInstance->GetGameHUD()->GetContextMenuAnchor())
					->SetPosition(UWidgetLayoutLibrary::GetMousePositionOnViewport(this));
			
			BGGameInstance->GetGameHUD()->ToggleContextMenu();
		}
	}
}

void ABGGamePlayerController::MoveCharacterToLocation()
{
	if (GrabbedActor && LastClickedActor)
	{
		auto const GrabbedBGCharacter = Cast<ABGCharacter>(GrabbedActor);
		if (GrabbedBGCharacter)
		{
			UE_LOG(LogTemp, Warning, TEXT("MoveTokenToLocation(): LastClickedActor = %s"), *LastClickedActor->GetName())

			FRotator const Rotation = FRotator(0.f, GrabbedBGCharacter->GetActorRotation().Yaw, 0.f);

			if (Cast<ABGActor>(LastClickedActor)->GetActorType() == EBGActorType::Tile)
			{
				float const ZedValue = 50.f;
				FVector Location;
				FVector ActorOrigin{};
				FVector ActorBoxExtent{};

				LastClickedActor->GetActorBounds(false, ActorOrigin, ActorBoxExtent, false);

				Location.X = ActorOrigin.X;
				Location.Y = ActorOrigin.Y;
				Location.Z = ZedValue + ActorOrigin.Z + ActorBoxExtent.Z;

				MoveCharacterToLocation_Server(GrabbedBGCharacter, Location, Rotation);
			}
		}
	}
}

void ABGGamePlayerController::HandleSplineStructureSelection()
{
	switch (ControlMode)
	{
	case EBGControlMode::Build:
		ModifySplineStructureLength();
		break;
	case EBGControlMode::Edit: break;
	case EBGControlMode::Move:
		/** Moving instanced static meshes over network doesn't seem to work */
		// MoveSplineStructure();
		break;
	default: ;
	}
}

void ABGGamePlayerController::SpawnSplineStructureAtLocation(FName const& WallStaticMeshName,
                                                             FName const& WallMaskedMaterialInstanceName,
                                                             FName const& CornerStaticMeshName,
                                                             FName const& CornerMaskedMaterialInstanceName,
                                                             FName const& BaseStaticMeshName,
                                                             FName const& BaseMaterialInstanceName,
                                                             FVector const& Location)
{
	SpawnSplineStructureAtLocation_Server(WallStaticMeshName, WallMaskedMaterialInstanceName,
	                                      CornerStaticMeshName, CornerMaskedMaterialInstanceName, BaseStaticMeshName,
	                                      BaseMaterialInstanceName, Location);
}

void ABGGamePlayerController::ModifySplineStructureLength()
{
	if (GrabbedActor && LastClickedActor)
	{
		auto const LastClickedBGActor = Cast<ABGActor>(LastClickedActor);
		auto const SplineComponent = Cast<UBGSplineWallComponent>(
			GrabbedActor->GetComponentByClass(UBGSplineWallComponent::StaticClass()));
		if (SplineComponent && LastClickedBGActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cursor Hit: %s"), *LastClickedActor->GetName())

			if (LastClickedBGActor->GetActorType() == EBGActorType::Tile)
			{
				/** Return if Tile is not visible */
				auto const TileComponent = Cast<UBGTileComponent>(LastClickedActor->GetComponentByClass(UBGTileComponent::StaticClass()));
				if (TileComponent && !TileComponent->GetTileVisibility())
				{
					return;
				}

				/** Get Tile's location, and match the Z axis to the current SplineStructure Z value */
				auto Location = LastClickedActor->GetActorLocation();
				Location.Z = GrabbedActor->GetActorLocation().Z;

				ModifyStructureLength_Server(SplineComponent, NearestIndexToClick, Location);
			}
		}
	}
}

float ABGGamePlayerController::GetClosestKeyOnSplineAtMousePosition(UBGSplineWallComponent* SplineComponent, FVector& OutIntersection) const
{
	FVector WorldPosition;
	FVector WorldDirection;
	DeprojectMousePositionToWorld(WorldPosition, WorldDirection);

	OutIntersection = FMath::LinePlaneIntersection(
		WorldPosition,
		WorldDirection * 2000.f + WorldPosition,
		SplineComponent->GetOwner()->GetActorLocation(),
		FVector(0.f, 0.f, 1.f)).GridSnap(100.f);

	/** On original Z-level */
	OutIntersection.Z = SplineComponent->GetOwner()->GetActorLocation().Z;

	return SplineComponent->GetSplineComponent()->FindInputKeyClosestToWorldLocation(OutIntersection);
}

void ABGGamePlayerController::AddSplinePointToSplineStructure(UBGSplineWallComponent* InSplineComponent)
{
	if (InSplineComponent)
	{
		if (NearestIndexToClick < 0)
		{
			FVector Intersection;
			auto const ClosestKey = GetClosestKeyOnSplineAtMousePosition(InSplineComponent, Intersection);

			const auto TotalSplinePoints = InSplineComponent->GetSplineComponent()->GetNumberOfSplinePoints();

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
				AddSplinePointToSplineStructure_Server(InSplineComponent, Intersection, NearestIndexToClick);
			}
		}
	}
}

void ABGGamePlayerController::MoveSplineStructure()
{
	if (GrabbedActor)
	{
		FVector Location{};

		UE_LOG(LogTemp, Warning, TEXT("Moving Structure"))

		LastHitResult.Reset();
		LastClickedActor = nullptr;
		if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel4), true,
		                                     LastHitResult))
		{
			auto const BGActor = Cast<ABGActor>(LastHitResult.GetActor());
			if (BGActor)
			{
				UE_LOG(LogTemp, Warning, TEXT("Cursor Hit: %s"), *LastHitResult.GetActor()->GetName())

				/** Return if Tile is not visible */
				if (BGActor->GetActorType() == EBGActorType::Tile)
				{
					auto TileComponent = Cast<UBGTileComponent>(BGActor->GetComponentByClass(UBGTileComponent::StaticClass()));
					if (TileComponent && !TileComponent->GetTileVisibility())
					{
						return;
					}

					LastClickedActor = Cast<ABGActor>(LastHitResult.GetActor());

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
			else
			{
				return;
			}
		}

		if (!HasAuthority())
		{
			GrabbedActor->SetActorLocation(Location);
		}
		
		MoveActorToLocation(Location);
	}
}

void ABGGamePlayerController::ModifySplineStructureInstanceMeshAtIndex(UBGSplineWallComponent* InSplineComponent,
                                                                       int const& Index,
                                                                       FString const& NewInstanceName,
                                                                       UStaticMesh* StaticMesh,
                                                                       UMaterialInstance* MaterialInstance,
                                                                       FString const& OldInstanceName)
{
	if (InSplineComponent)
	{
		// Just go straight to server call
		ModifyInstanceMeshAtIndex_Server(InSplineComponent, Index, NewInstanceName, StaticMesh,
		                                 MaterialInstance, OldInstanceName);
	}
}

void ABGGamePlayerController::SpawnStructureActorAtSplineStructureIndex(UBGSplineWallComponent* InSplineComponent,
                                                                        int const& Index,
                                                                        TSubclassOf<ABGStructure> StructureClassToSpawn,
                                                                        FString const& OldInstanceName)
{
	if (InSplineComponent && StructureClassToSpawn.Get())
	{
		SpawnStructureActorAtSplineStructureIndex_Server(InSplineComponent, Index, StructureClassToSpawn,
		                                                 OldInstanceName);
	}
}

void ABGGamePlayerController::RemoveInstanceAtIndexOnSplineStructure(UBGSplineWallComponent* InSplineComponent,
                                                                     int const& Index, FString const& InstanceName)
{
	if (InSplineComponent)
	{
		RemoveInstanceAtIndexOnSplineStructure_Server(InSplineComponent, Index, InstanceName);
	}
}

void ABGGamePlayerController::RestoreInstanceAtIndexOnSplineStructure(UBGSplineWallComponent* InSplineComponent,
                                                                      int const& Index,
                                                                      FTransform const& NewInstanceTransform,
                                                                      FString const& InstanceName)
{
	if (InSplineComponent)
	{
		RestoreInstanceAtIndexOnSplineStructure_Server(InSplineComponent, Index, NewInstanceTransform, InstanceName);
	}
}

void ABGGamePlayerController::DestroyStructureActor(ABGStructure* StructureToRemove)
{
	if (StructureToRemove)
	{
		DestroyStructureActor_Server(StructureToRemove);
	}
}

// void ABGGamePlayerController::ToggleLockSplineStructureInPlace(ABGActor* SplineStructureToLock,
//                                                                bool const bNewLocked)
// {
// 	if (SplineStructureToLock)
// 	{
// 		ToggleLockSplineStructureInPlace_Server(SplineStructureToLock, bNewLocked);
// 	}
// }
//
// void ABGGamePlayerController::ToggleLockStructure(ABGStructure* StructureToLock, bool const bNewLocked)
// {
// 	if (StructureToLock)
// 	{
// 		ToggleLockStructure_Server(StructureToLock, bNewLocked);
// 	}
// }

void ABGGamePlayerController::ToggleDoorOpenClose(ABGDoor* DoorToToggle)
{
	if (DoorToToggle)
	{
		ToggleDoorOpenClose_Server(DoorToToggle);
	}
}

void ABGGamePlayerController::ResetSplineStructure(UBGSplineWallComponent* InSplineComponent) const
{
	if (InSplineComponent)
	{
		if (!HasAuthority())
		{
			if (InSplineComponent->GetSplineComponent()->GetNumberOfSplinePoints() > 2)
			{
				// clear all spline points except for indices 0 and 1
				for (int i{InSplineComponent->GetSplineComponent()->GetNumberOfSplinePoints()}; i > 2; --i)
				{
					InSplineComponent->GetSplineComponent()->RemoveSplinePoint(i - 1);
				}
			}

			// reset Spline Point 0 to actor's origin
			InSplineComponent->GetSplineComponent()->SetLocationAtSplinePoint(
				0, InSplineComponent->GetOwner()->GetActorLocation(), ESplineCoordinateSpace::World, true);

			// reset Spline Point 1 to 105.f away the origin
			auto Location = InSplineComponent->GetOwner()->GetActorLocation();
			Location.X += 50.f;

			InSplineComponent->GetSplineComponent()->SetLocationAtSplinePoint(
				1, Location, ESplineCoordinateSpace::World, true);

			InSplineComponent->UpdateSplineStructureMesh();
		}
	}
}

// void ABGGamePlayerController::HandleActorSelection()
// {
// 	/** Removed GameMasterPermission check at this time because of improper Server Defaulting */
// 	
// 	// if (!GetGameMasterPermissions())
// 	// {
// 	// 	return;
// 	// }
// 	
// 	if (GrabbedActor && LastClickedActor)
// 	{
// 		UE_LOG(LogTemp, Warning, TEXT("HandleActorSelection(): LastClickedActor = %s"), *LastClickedActor->GetName())
// 		FVector Location;
//
// 		if (Cast<ABGActor>(LastClickedActor)->GetActorType() == EBGActorType::Structure) return;
//
// 		FVector ActorOrigin{};
// 		FVector ActorBoxExtent{};
//
// 		LastClickedActor->GetActorBounds(false, ActorOrigin, ActorBoxExtent, false);
//
// 		Location.X = ActorOrigin.X;
// 		Location.Y = ActorOrigin.Y;
// 		/** Eventually don't hardcode 300 here...make it better */
// 		Location.Z = 150.f + ActorOrigin.Z + ActorBoxExtent.Z;
//
// 		MoveActorToLocation(Location);
// 	}
//
// }

void ABGGamePlayerController::MoveActorToLocation(FVector const& Location)
{
	if (GrabbedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("Grabbed Actor Is Valid"))
		if (!HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("We don't have network authority"))

			GrabbedActor->SetActorLocation(Location);
		}
		MoveActorToLocation_Server(GrabbedActor, Location);
	}
}

void ABGGamePlayerController::ToggleActorLockedState(ABGActor* InActor, bool const bSetLocked)
{
	if (InActor)
	{
		ToggleActorLockedState_Server(InActor, bSetLocked);
	}
}

void ABGGamePlayerController::ToggleTileVisibility(UBGTileComponent* TileToToggle)
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

void ABGGamePlayerController::ShrinkBoard(UBGBoardComponent* BoardToShrink)
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

void ABGGamePlayerController::GrowBoard(UBGBoardComponent* BoardToGrow)
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

void ABGGamePlayerController::SpawnNewActor_Server_Implementation(TSubclassOf<ABGActor> ActorToSpawn)
{
	if (ActorToSpawn.GetDefaultObject())
	{
		Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this))->SpawnNewActor(ActorToSpawn);
	}
}

void ABGGamePlayerController::ToggleActorLockedState_Server_Implementation(ABGActor* InActor, bool const bSetLocked)
{
	if (InActor)
	{
		InActor->ToggleLocked(bSetLocked);
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

// void ABGGamePlayerController::ToggleLockStructure_Server_Implementation(
// 	ABGStructure* StructureToLock, bool const bNewLocked)
// {
// 	if (HasAuthority() && StructureToLock)
// 	{
// 		StructureToLock->ToggleStructureLock(bNewLocked);
// 	}
// }

void ABGGamePlayerController::RestoreInstanceAtIndexOnSplineStructure_Server_Implementation(
	UBGSplineWallComponent* InSplineComponent, int const& Index, FTransform const& NewInstanceTransform,
	FString const& InstanceName)
{
	if (HasAuthority() && InSplineComponent)
	{
		InSplineComponent->RestoreInstanceMeshAtIndex(Index, NewInstanceTransform, InstanceName);
	}
}

void ABGGamePlayerController::RemoveInstanceAtIndexOnSplineStructure_Server_Implementation(
	UBGSplineWallComponent* InSplineComponent, int const& Index, FString const& InstanceName)
{
	if (HasAuthority() && InSplineComponent)
	{
		InSplineComponent->RemoveInstanceMeshAtIndex(Index, InstanceName);
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
	UBGSplineWallComponent* InSplineComponent, int const& Index,
	TSubclassOf<ABGStructure> StructureClassToSpawn,
	FString const& OldInstanceName)
{
	if (HasAuthority() && InSplineComponent && StructureClassToSpawn.Get())
	{
		ABGGameplayGameModeBase::SpawnStructureActorAtSplineStructureIndex(
			InSplineComponent, Index, StructureClassToSpawn, OldInstanceName);
	}
}

// void ABGGamePlayerController::ToggleLockSplineStructureInPlace_Server_Implementation(
// 	ABGActor* SplineStructureToLock,
// 	bool const bNewLocked)
// {
// 	if (HasAuthority() && SplineStructureToLock)
// 	{
// 		auto SplineWallComponent = SplineStructureToLock->GetComponentByClass(UBGSplineWallComponent::StaticClass());
// 		if (SplineWallComponent)
// 		{
// 			SplineWallComponent->
// 		}
// 		SplineStructureToLock->ToggleLockStructureInPlace(bNewLocked);
// 	}
// }

void ABGGamePlayerController::ModifyInstanceMeshAtIndex_Server_Implementation(
	UBGSplineWallComponent* InSplineComponent, int const& Index, FString const& NewInstanceName, UStaticMesh* StaticMesh,
	UMaterialInstance* MaterialInstance, FString const& OldInstanceName)
{
	if (HasAuthority() && InSplineComponent)
	{
		ABGGameplayGameModeBase::ModifySplineStructureInstanceMeshAtIndex(InSplineComponent, Index, NewInstanceName,
		                                                                  StaticMesh,
		                                                                  MaterialInstance, OldInstanceName);
	}
}

void ABGGamePlayerController::ResetSplineStructure_Server_Implementation(UBGSplineWallComponent* InSplineComponent)
{
	if (HasAuthority() && InSplineComponent)
	{
		ABGGameplayGameModeBase::ResetSplineStructure(InSplineComponent);
	}
}

void ABGGamePlayerController::ShrinkBoard_Server_Implementation(UBGBoardComponent* BoardToShrink)
{
	if (HasAuthority() && BoardToShrink)
	{
		ABGGameplayGameModeBase::ShrinkBoard(BoardToShrink);
	}
}

void ABGGamePlayerController::GrowBoard_Server_Implementation(UBGBoardComponent* BoardToGrow)
{
	if (HasAuthority() && BoardToGrow)
	{
		ABGGameplayGameModeBase::GrowBoard(BoardToGrow);
	}
}

void ABGGamePlayerController::SpawnSplineStructureAtLocation_Server_Implementation(
	FName const& WallStaticMeshName,
	FName const& WallMaskedMaterialInstanceName,
	FName const& CornerStaticMeshName,
	FName const& CornerMaskedMaterialInstanceName,
	FName const& BaseStaticMeshName,
	FName const& BaseMaterialInstanceName,
	FVector const& Location)
{
	if (HasAuthority())
	{
		/** Using Factory to produce BGActor w/ spline wall component */
		auto CastGameMode = Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this));
		if (CastGameMode)
		{
			FBGStructureInfo NewStructureInfo;
			CastGameMode->CreateStructureInfo(NewStructureInfo, WallStaticMeshName,
				WallMaskedMaterialInstanceName, CornerStaticMeshName, CornerMaskedMaterialInstanceName,
				BaseStaticMeshName, BaseMaterialInstanceName, Location);
			
			auto const NewSplineWall = CastGameMode->CreateNewActor(EBGActorType::Structure);
			if (NewSplineWall)
			{
				auto SplineComponent = Cast<UBGSplineWallComponent>(NewSplineWall->
					GetComponentByClass(UBGSplineWallComponent::StaticClass()));
				if (SplineComponent)
				{
					SplineComponent->SetStructureInfo(NewStructureInfo);
					FTransform SpawnTransform;
					SpawnTransform.SetLocation(Location);
					NewSplineWall->FinishSpawning(SpawnTransform);
					SplineComponent->Initialize();
					UE_LOG(LogTemp, Warning, TEXT("Spawning Structure At Location (server)"))
				}
			}
		}
	}
}

void ABGGamePlayerController::SpawnNewBoard_Server_Implementation(int const& Zed, int const& X, int const& Y)
{
	if (HasAuthority())
	{
		Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this))->SpawnNewBoard(Zed, X, Y);
	}
}

void ABGGamePlayerController::ToggleCharacterPermissionsForPlayer_Server_Implementation(ABGPlayerState* PlayerStateToToggle,
	ABGCharacter* CharacterToToggle)
{
	if (HasAuthority() && PlayerStateToToggle && CharacterToToggle)
	{
		ABGGameplayGameModeBase::ToggleCharacterPermissionsForPlayer(PlayerStateToToggle, CharacterToToggle);
	}
}

void ABGGamePlayerController::RotateCharacter_Server_Implementation(ABGCharacter* CharacterToRotate, FRotator const& NewRotation)
{
	if (HasAuthority() && CharacterToRotate)
	{
		CharacterToRotate->SetActorRotation(NewRotation);
	}
}

void ABGGamePlayerController::ResetCharacterRotation_Server_Implementation(ABGCharacter* CharacterToReset)
{
	if (HasAuthority() && CharacterToReset)
	{
		CharacterToReset->SetActorRotation(FRotator::ZeroRotator);
	}
}

void ABGGamePlayerController::ToggleCharacterLockInPlace_Server_Implementation(ABGCharacter* CharacterToToggle, bool bLock)
{
	if (HasAuthority() && CharacterToToggle)
	{
		ABGGameplayGameModeBase::ToggleCharacterLockInPlace(CharacterToToggle, bLock);
	}
}

void ABGGamePlayerController::SpawnCharacterAtLocation_Server_Implementation(
	FVector const& Location, FName const& MeshName, FName const& MaterialName)
{
	if (HasAuthority())
	{
		Cast<ABGGameplayGameModeBase>(UGameplayStatics::GetGameMode(this))->SpawnCharacterAtLocation(
			Location, MeshName, MaterialName);

		UE_LOG(LogTemp, Warning, TEXT("Spawning Token At Location (server)"))
	}
}

void ABGGamePlayerController::MoveCharacterToLocation_Server_Implementation(ABGCharacter* CharacterToMove, FVector const& Location,
                                                                        FRotator const TokenRotation)
{
	if (HasAuthority() && CharacterToMove)
	{
		UE_LOG(LogTemp, Warning, TEXT("Moving Token To Location (server)"))
		ABGGameplayGameModeBase::MoveCharacterToLocation(CharacterToMove, Location, TokenRotation);
	}
}

void ABGGamePlayerController::ModifyStructureLength_Server_Implementation(
	UBGSplineWallComponent* InSplineComponent, int const& PointIndex,
	FVector const& NewLocation)
{
	if (InSplineComponent)
	{
		ABGGameplayGameModeBase::ModifySplineStructureLength(InSplineComponent, PointIndex, NewLocation);
	}
}

void ABGGamePlayerController::AddSplinePointToSplineStructure_Server_Implementation(
	UBGSplineWallComponent* InSplineComponent,
	FVector const& ClickLocation, int const& Index)
{
	if (HasAuthority() && InSplineComponent)
	{
		ABGGameplayGameModeBase::AddSplinePointToSplineStructure(InSplineComponent, ClickLocation,
		                                                         Index);
	}
}

void ABGGamePlayerController::RotateCharacter(float Value)
{
	if (Value != 0)
	{
		if (GrabbedActor)
		{
			auto const GrabbedBGCharacter = Cast<ABGCharacter>(GrabbedActor);
			if (GrabbedBGCharacter)
			{
				auto const OriginalRotation = GrabbedBGCharacter->GetActorRotation().GetDenormalized();
				float const Remainder = FMath::Fmod(OriginalRotation.Yaw, 45.f);

				/** If we have a Yaw that is greater than or equal to 360 degrees, use 0 instead */
				int Quotient = (OriginalRotation.Yaw > 337.5f ? 0 : OriginalRotation.Yaw) / 45;

				// UE_LOG(LogTemp, Warning, TEXT("Incoming Yaw: %f"), OriginalRotation.Yaw)
				// UE_LOG(LogTemp, Warning, TEXT("Remainder: %f"), Remainder)
				// UE_LOG(LogTemp, Warning, TEXT("Quotient: %d"), Quotient)

				/** 
				 * if our Yaw is close to 360 then don't upgrade the Quotient
				 * (lest we shoot past 45 and go to 90)
				 */
				if (Remainder >= 22.5f && OriginalRotation.Yaw < 337.5f)
				{
					++Quotient;
				}

				Quotient *= 45;

				if (Value > 0)
				{
					Quotient += 45;
				}
				else if (Value < 0)
				{
					Quotient -= 45;
					Quotient < 0 ? Quotient += 360 : Quotient;
				}

				// UE_LOG(LogTemp, Warning, TEXT("New Angle: %d"), Quotient)

				auto NewRotation = FRotator(0.f, Quotient, 0.f);
				NewRotation.Normalize();

				if (!HasAuthority())
				{
					GrabbedBGCharacter->SetActorRotation(NewRotation);
				}
				RotateCharacter_Server(GrabbedBGCharacter, NewRotation);
			}
		}
	}
}

void ABGGamePlayerController::ResetCharacterRotation(ABGCharacter* CharacterToReset)
{
	if (CharacterToReset)
	{
		if (!HasAuthority())
		{
			CharacterToReset->SetActorRotation(FRotator::ZeroRotator);
		}
		ResetCharacterRotation_Server(CharacterToReset);
	}
}

void ABGGamePlayerController::ToggleCharacterLockInPlace(ABGCharacter* CharacterToToggle, bool bLock)
{
	if (CharacterToToggle)
	{
		if (!HasAuthority())
		{
			CharacterToToggle->ToggleLockTokenInPlace(bLock);
		}
		ToggleCharacterLockInPlace_Server(CharacterToToggle, bLock);
	}
}

void ABGGamePlayerController::ToggleCharacterPermissionsForPlayer(ABGPlayerState* PlayerStateToToggle,
                                                              ABGCharacter* CharacterToToggle)
{
	if (PlayerStateToToggle && CharacterToToggle)
	{
		if (!HasAuthority())
		{
			CharacterToToggle->PlayerHasPermissions(PlayerStateToToggle)
				? CharacterToToggle->RemovePlayerFromPermissionsArray(PlayerStateToToggle)
				: CharacterToToggle->AddPlayerToPermissionsArray(PlayerStateToToggle);
		}

		ToggleCharacterPermissionsForPlayer_Server(PlayerStateToToggle, CharacterToToggle);
	}
}

void ABGGamePlayerController::MoveActorToLocation_Server_Implementation(AActor* ActorToMove,
                                                                        FVector const& NewLocation)
{
	if (ActorToMove)
	{
		UE_LOG(LogTemp, Warning, TEXT("Server: Moving Actor to Location"))

		ABGGameplayGameModeBase::MoveActorToLocation(ActorToMove, FTransform(NewLocation));
	}
}

void ABGGamePlayerController::ToggleTileVisibility_Server_Implementation(UBGTileComponent* TileToToggle)
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