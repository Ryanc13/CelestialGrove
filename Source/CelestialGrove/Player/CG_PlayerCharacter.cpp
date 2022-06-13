// ============================================================
// FILE: CG_PlayerCharacter.cpp
// AUTHOR: RyanC
// ============================================================

#include "CG_PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "CG_InteractableBase.h"
#include "CG_GlobalDefines.h"
#include "GameFramework/PlayerController.h"

// -----------------------------------------------------------------------------------------
ACG_PlayerCharacter::ACG_PlayerCharacter()
{
	// ============================================================
	// Tick settings
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// ============================================================
	// Create components
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->AddRelativeLocation(FVector(-10.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	InspectionAnchorPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Inspection Target Component"));
	InspectionAnchorPoint->SetupAttachment(FirstPersonCamera);
	InspectionAnchorPoint->AddRelativeLocation(FVector(80.0f, 0.0f, 0.0f));

	// ============================================================
	// Default initializations
	InspectionTarget = nullptr;
	CurrentState = EPlayerState::DEFAULT;
	
	InteractionDistance = 150.0f;
	InspectionInterpSpeed = 25.0f;
	InspectionRotationSpeed = FVector2D(50.f, 100.f);
	InspectionAxisTolerance = 2.5f;

	isDraggingForInspection = false;

	MouseSensitivity = FVector2D(1.f, 1.f);
	ThrowStrength = 800.f;
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACG_PlayerCharacter::Interact);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACG_PlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACG_PlayerCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &ACG_PlayerCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ACG_PlayerCharacter::LookUp);
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	switch(CurrentState)
	{
		// ============================================================
		case EPlayerState::INSPECTING:
		{
			checkf(InspectionTarget, TEXT("In inspection state without a valid inspection target!"));
			FVector anchorPos = InspectionAnchorPoint->GetComponentLocation();

			// ============================================================
			// Before we start handling the inspection specific game logic we need to make sure that the object has
			// reached the inspection point, if not then interpolate to that position over time.
			FVector curPosition = InspectionTarget->GetActorLocation();
			FVector targetPosition =  anchorPos + InspectionTarget->InspectionOffset();
			FVector displacementVector = targetPosition - curPosition;

			if (!displacementVector.IsNearlyZero(0.15f))
			{
				FVector newPos = FMath::VInterpTo(curPosition, targetPosition, deltaTime, InspectionInterpSpeed);
				InspectionTarget->SetActorLocation(newPos);
			}
			else if (isDraggingForInspection)
			{
				// ============================================================
				// Using the mouse deltas we are going to try and determine the intended axis of rotation
				// when the different is above the tolerance then specifically only rotate on the 
				MouseDelta.Normalize();
				float absX = FMath::Abs<float>(MouseDelta.X);
				float absY = FMath::Abs<float>(MouseDelta.Y);
				float directionDiff = FMath::Abs<float>(absX - absY);
				if (directionDiff >= InspectionAxisTolerance)
				{
					if (absX > absY)
					{
						MouseDelta.Y = 0.f;
					}
					else
					{
						MouseDelta.X = 0.f;
					}
				}

				MouseDelta *= InspectionRotationSpeed * deltaTime;
				FRotator toRotate(-MouseDelta.Y, -MouseDelta.X, 0.0f);

				FVector rotatedAnchorToObject = toRotate.RotateVector(curPosition - anchorPos);
				FVector newLocation = anchorPos + rotatedAnchorToObject;

				InspectionTarget->SetActorLocation(newLocation);
				InspectionTarget->AddActorWorldRotation(toRotate);
			}

			MouseDelta.X = 0.f;
			MouseDelta.Y = 0.f;
		}
		break;
	}
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::Interact()
{
	if (CurrentState == EPlayerState::DEFAULT)
	{
		FHitResult result;

		FVector startOfTrace = FirstPersonCamera->GetComponentLocation();
		FVector endOfTrace = startOfTrace + (FirstPersonCamera->GetForwardVector() * InteractionDistance);

		FCollisionQueryParams params = FCollisionQueryParams::DefaultQueryParam;
		params.TraceTag = TEXT("Interaction Trace");
		params.OwnerTag = GetFName(); 
		params.AddIgnoredActor(this);

		bool wasHit = GetWorld()->LineTraceSingleByChannel(
															result,
															startOfTrace,
															endOfTrace,
															INTERACTABLE_COLLISION_CHANNEL,
															params
														  );

		if (wasHit)
		{
			ACG_InteractableBase * interactable = Cast<ACG_InteractableBase>(result.GetActor());
			if (interactable)
			{
				interactable->OnInteracted(this);
			}
		}
	}
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::StartInspectionRotation()
{
	if (CurrentState == EPlayerState::INSPECTING)
	{
		isDraggingForInspection = true;
		UpdateCursor(isDraggingForInspection);
	}
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::StopInspectionRotation()
{
	if (CurrentState == EPlayerState::INSPECTING)
	{
		isDraggingForInspection = false;
		UpdateCursor(isDraggingForInspection);
	}
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::MoveForward(float value)
{
	if (value != 0.0f && !IsMovementDisabled())
	{
		AddMovementInput(GetActorForwardVector(), value);
	}
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::MoveRight(float value)
{
	if (value != 0.0f && !IsMovementDisabled())
	{
		AddMovementInput(GetActorRightVector(), value);
	}
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::Turn(float rate)
{
	if (rate != 0.0f && !IsMovementDisabled())
	{
		AddControllerYawInput(rate * MouseSensitivity.X);
	}
	else if (CurrentState == EPlayerState::INSPECTING)
	{
		MouseDelta.X = rate;
	}
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::LookUp(float rate)
{
	if (rate != 0.0f && !IsMovementDisabled())
	{
		AddControllerPitchInput(rate * MouseSensitivity.Y);
	}
	else if (CurrentState == EPlayerState::INSPECTING)
	{
		MouseDelta.Y = rate;
	}
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::BeginInspection(ACG_InteractableBase * const interactable)
{
	InspectionTarget = interactable;
	InspectionTarget->AttachToComponent(InspectionAnchorPoint, FAttachmentTransformRules::KeepWorldTransform);

	CurrentState = EPlayerState::INSPECTING;

	InputComponent->BindAction("QuitInspection", IE_Pressed, this, &ACG_PlayerCharacter::InspectionEnded);
	InputComponent->BindAction("InspectionDrag", IE_Pressed, this, &ACG_PlayerCharacter::StartInspectionRotation);
	InputComponent->BindAction("InspectionDrag", IE_Released, this, &ACG_PlayerCharacter::StopInspectionRotation);

	SetMouseCursorShown(true);

	SetActorTickEnabled(true);
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::InspectionEnded()
{
	check(CurrentState == EPlayerState::INSPECTING);
	check(InspectionTarget);

	InputComponent->RemoveActionBinding("QuitInspection", IE_Pressed);
	InputComponent->RemoveActionBinding("InspectionDrag", IE_Pressed);
	InputComponent->RemoveActionBinding("InspectionDrag", IE_Released);

	InspectionTarget->OnEndInspection(FirstPersonCamera->GetForwardVector(), ThrowStrength);
	InspectionTarget = nullptr;
	CurrentState = EPlayerState::DEFAULT;

	SetMouseCursorShown(false);

	SetActorTickEnabled(false);
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::SetMouseCursorShown(bool isShown)
{
	APlayerController * playerController = Cast<APlayerController>(GetController());
	check(playerController);
	playerController->bShowMouseCursor = isShown;
	playerController->bEnableClickEvents = isShown;
	playerController->bEnableMouseOverEvents = isShown;
	UpdateCursor(false);
	ChangeHUD(CurrentState);

	if (isShown)
	{
		playerController->SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		playerController->SetInputMode(FInputModeGameOnly());
	}
}
