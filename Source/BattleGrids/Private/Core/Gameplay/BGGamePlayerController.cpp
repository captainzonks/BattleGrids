// © 2021 Matthew Barham. All Rights Reserved.

#include "Core/Gameplay/BGGamePlayerController.h"

#include "Actors/BGBoard.h"
#include "Actors/BGSplineStructure.h"
#include "Actors/BGTile.h"
#include "Actors/BGToken.h"
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
	case EBGGrabbedObjectType::None: break;
	case EBGGrabbedObjectType::Token:
		HandleTokenSelection();
		break;
	case EBGGrabbedObjectType::Structure:
		HandleStructureSelection();
		break;
	case EBGGrabbedObjectType::Board: break;
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

	// DOREPLIFETIME(ABGGamePlayerController, ControlMode)
	// DOREPLIFETIME(ABGGamePlayerController, GrabbedObject)
	// DOREPLIFETIME(ABGGamePlayerController, GrabbedToken)
	// DOREPLIFETIME(ABGGamePlayerController, GrabbedStructure)
	// DOREPLIFETIME(ABGGamePlayerController, GrabbedBoard)
	// DOREPLIFETIME(ABGGamePlayerController, LastTargetedActor)
}

void ABGGamePlayerController::SelectObject()
{
	FHitResult HitResult;
	GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, HitResult);

	if (HitResult.bBlockingHit && HitResult.GetActor()->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitResult.GetActor()->GetName())

		if ((GrabbedToken = Cast<ABGToken>(HitResult.GetActor()))->IsValidLowLevel())
			GrabbedObject = EBGGrabbedObjectType::Token;

		if ((GrabbedStructure = Cast<ABGSplineStructure>(HitResult.GetActor()))->IsValidLowLevel())
			GrabbedObject = EBGGrabbedObjectType::Structure;

		if ((GrabbedBoard = Cast<ABGBoard>(HitResult.GetActor()))->IsValidLowLevel())
			GrabbedObject = EBGGrabbedObjectType::Board;
	}
}

void ABGGamePlayerController::ReleaseObject()
{
	GrabbedObject = EBGGrabbedObjectType::None;

	if (GrabbedToken)
	{
		if (LastTargetedActor)
		{
			MoveTokenToLocation(false);
			SetTokenCollisionAndPhysics_Server(GrabbedToken, true, true, ECollisionEnabled::Type::QueryAndPhysics);
		}
	}

	GrabbedToken = nullptr;
	GrabbedStructure = nullptr;
	GrabbedBoard = nullptr;
	LastTargetedActor = nullptr;
	NearestIndexToClick = -1;
}

void ABGGamePlayerController::HandleTokenSelection()
{
	if (GrabbedToken)
	{
		if (!GetGameMasterPermissions() && !GrabbedToken->PlayerHasPermissions(GetPlayerState<ABGPlayerState>()))
			return;

		SetTokenCollisionAndPhysics_Server(GrabbedToken, true, false, ECollisionEnabled::Type::PhysicsOnly);

		FHitResult HitResult;
		if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, HitResult))
		{
			if (HitResult.GetActor()->IsValidLowLevel())
			{
				UE_LOG(LogTemp, Warning, TEXT("Cursor Hit: %s"), *HitResult.GetActor()->GetName())

				if (HitResult.GetActor()->GetClass() == ABGTile::StaticClass())
				{
					if (!Cast<ABGTile>(HitResult.GetActor())->GetStaticMeshComponent()->GetVisibleFlag())
						return;
				}

				LastTargetedActor = HitResult.GetActor();
				MoveTokenToLocation(true);
			}
		}
	}
}

void ABGGamePlayerController::MoveTokenToLocation(bool const bHolding)
{
	if (GrabbedToken)
	{
		FRotator const Rotation = FRotator(0.f, GrabbedToken->GetActorRotation().Yaw, 0.f);

		// Move the token locally if we are a client
		if (!HasAuthority())
		{
			float ZedValue;
			bHolding ? ZedValue = 100.f : ZedValue = 50.f;

			FVector ActorOrigin{};
			FVector ActorBoxExtent{};
			LastTargetedActor->GetActorBounds(false, ActorOrigin, ActorBoxExtent, false);

			FVector Location;
			Location.X = ActorOrigin.X;
			Location.Y = ActorOrigin.Y;
			Location.Z = ZedValue + ActorOrigin.Z + ActorBoxExtent.Z;

			GrabbedToken->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
			GrabbedToken->SetActorRotation(Rotation, ETeleportType::TeleportPhysics);
		}

		// Make a server call to ask the GameMode to move the token
		MoveTokenToLocation_Server(GrabbedToken, LastTargetedActor, bHolding, Rotation);
	}
}

void ABGGamePlayerController::HandleStructureSelection()
{
	if (!GetGameMasterPermissions())
		return;

	if (GetInputAnalogKeyState(EKeys::LeftAlt) == 1)
		AddSplinePointToStructureSpline();
	ModifyStructureLength();
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
		                                                            FVector(0.f, 0.f, 1.f)).GridSnap(105.f);

		// Offset the intersection by 50 in the X and Y to move to the center of the grid squares, and set the Z equal to original structure;
		GridSnappedIntersection.X -= 50.f;
		GridSnappedIntersection.Y -= 50.f;
		GridSnappedIntersection.Z = GrabbedStructure->GetActorLocation().Z;

		if (NearestIndexToClick == -1)
			NearestIndexToClick = FMath::RoundToInt(
				GrabbedStructure->GetSplineComponent()->FindInputKeyClosestToWorldLocation(GridSnappedIntersection));

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
			auto const Intersection = FMath::LinePlaneIntersection(
				WorldPosition,
				WorldDirection * 2000.f + WorldPosition,
				GrabbedStructure->GetActorLocation(),
				FVector(0.f, 0.f, 1.f));

			// Insert a new spline point at an index found near the intersection
			AddSplinePointToStructureSpline_Server(GrabbedStructure, GrabbedStructure->GetSplineComponent()->
			                                       FindLocationClosestToWorldLocation(
				                                       Intersection,
				                                       ESplineCoordinateSpace::World),
			                                       FMath::RoundToInt(GrabbedStructure->
			                                                         GetSplineComponent()->
			                                                         FindInputKeyClosestToWorldLocation(
				                                                         Intersection)
			                                       ));
		}

		ModifyStructureLength();
	}
}

void ABGGamePlayerController::MoveTokenToLocation_Server_Implementation(ABGToken* TokenToMove, AActor* TargetActor,
                                                                        bool const bHolding,
                                                                        FRotator const TokenRotation)
{
	ABGGameplayGameModeBase::MoveTokenToLocation(TokenToMove, TargetActor, bHolding, TokenRotation);
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
		ABGGameplayGameModeBase::AddSplinePointToStructureSpline(StructureToModify, ClickLocation, Index);
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
			GrabbedToken->SetActorRotation(NewRotation);
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
