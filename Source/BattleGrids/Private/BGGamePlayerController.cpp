// © 2021 Matthew Barham. All Rights Reserved.


#include "BGGamePlayerController.h"

#include "BGToken.h"

void ABGGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("RotateToken", this, &ABGGamePlayerController::RotateToken);
}

void ABGGamePlayerController::RotateToken(float Value)
{
	if (Value != 0)
	{
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
			
			UE_LOG(LogTemp, Warning, TEXT("New Angle: %d"), Quotient)

			auto NewRotation = FRotator(0.f, Quotient, 0.f);
			NewRotation.Normalize();
			GrabbedToken->SetActorRotation(NewRotation);
		}
	}
}

void ABGGamePlayerController::UpdateTransformOnServer_Implementation(FTransform NewTransform)
{
	GetPawn()->SetActorTransform(NewTransform);
}
