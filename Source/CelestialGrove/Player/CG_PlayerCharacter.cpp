// ============================================================
// FILE: CG_PlayerCharacter.cpp
// AUTHOR: RyanC
// ============================================================

#include "CG_PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "CG_InteractableBase.h"
#include "CG_EnemyCharacter.h"
#include "GameFramework/PlayerController.h"
#include "CG_SpellBase.h"

// -----------------------------------------------------------------------------------------
ACG_PlayerCharacter::ACG_PlayerCharacter()
{
	// ============================================================
	// Tick settings
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

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

	MovementSpeed = 120.f;

	ActiveSpell = 0;
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	Target.OwningActor = this;
	Target.ApplyDamageDelegate.AddUObject(this, &ACG_PlayerCharacter::ApplyDamage);
	Target.ApplyStatusDelegate.AddUObject(this, &ACG_PlayerCharacter::ApplyStatus);
	Target.ApplyForceDelegate.AddUObject(this, &ACG_PlayerCharacter::ApplyForce);
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACG_PlayerCharacter::Interact);
	PlayerInputComponent->BindAction("CastSpell", IE_Pressed, this, &ACG_PlayerCharacter::CastSpell);

	PlayerInputComponent->BindAxis("MoveForward", this, &ACG_PlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACG_PlayerCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &ACG_PlayerCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ACG_PlayerCharacter::LookUp);
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	for (int32 i = 0; i < EquippedSpells.Num(); ++i)
	{
		EquippedSpells[i]->UpdateSpell(deltaTime, this);
	}

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
				// when the difference is above the tolerance then specifically only rotate on the intended axis.
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
				FVector xRotation = GetActorUpVector() * -MouseDelta.X;
				FVector yRotation = FirstPersonCamera->GetRightVector() * MouseDelta.Y;
				FVector inspectSpaceDelta = xRotation + yRotation;
				FRotator toRotate(inspectSpaceDelta.Y, inspectSpaceDelta.Z, inspectSpaceDelta.X);

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
bool ACG_PlayerCharacter::TraceForCollision(FHitResult & result, float distance) const
{
	FVector startOfTrace = FirstPersonCamera->GetComponentLocation();
	FVector endOfTrace = startOfTrace + (FirstPersonCamera->GetForwardVector() * distance);

	FCollisionQueryParams params = FCollisionQueryParams::DefaultQueryParam;
	params.TraceTag = TEXT("Spell Trace");
	params.OwnerTag = GetFName(); 
	params.AddIgnoredActor(this);

	return GetWorld()->LineTraceSingleByChannel(
												result,
												startOfTrace,
												endOfTrace,
												SPELL_TRACE_CHANNEL,
												params
											   );
}

// -----------------------------------------------------------------------------------------
bool ACG_PlayerCharacter::GetTargetsInSphere(ESpellCollisionType type, float radius, FVector & location, TArray<FCG_SpellTarget> & targets) const
{
	TArray<FOverlapResult> overlaps;

	FCollisionObjectQueryParams objParams;
	switch(type)
	{
		case ESpellCollisionType::ALL:
			objParams = FCollisionObjectQueryParams(SPELL_TRACE_CHANNEL);
		break;

		case ESpellCollisionType::INANIMATE_ONLY:
			objParams = FCollisionObjectQueryParams(INANIMATE_COLLISION_CHANNEL);
		break;

		case ESpellCollisionType::ANIMATE_ONLY:
			objParams = FCollisionObjectQueryParams(ANIMATE_COLLISION_CHANNEL);
		break;
	}

	FCollisionShape shape {};
	shape.SetSphere(radius);

	FCollisionQueryParams params = FCollisionQueryParams::DefaultQueryParam;
	params.TraceTag = TEXT("Interaction Trace");
	params.OwnerTag = GetFName(); 
	params.AddIgnoredActor(this);

	bool res = GetWorld()->OverlapMultiByObjectType(
													overlaps,
													location,
													FQuat::Identity,
													objParams,
													shape,
													params
												   );
	
	if (res)
	{
		for (const FOverlapResult & overlap : overlaps)
		{
			if ((type == ESpellCollisionType::ALL || type == ESpellCollisionType::INANIMATE_ONLY) &&
				overlap.GetActor()->GetClass() == ACG_InteractableBase::StaticClass())
			{
				ACG_InteractableBase * interactable = Cast<ACG_InteractableBase>(overlap.GetActor());
				interactable->Target.ImpactDirection = (interactable->GetCenterOfMass() - location);
				interactable->Target.ImpactDirection.Normalize();

				targets.Emplace(interactable->Target);
			}
			else if ((type == ESpellCollisionType::ALL || type == ESpellCollisionType::ANIMATE_ONLY) &&
					overlap.GetActor()->GetClass() == ACG_EnemyCharacter::StaticClass())
			{
				ACG_EnemyCharacter * enemy = Cast<ACG_EnemyCharacter>(overlap.GetActor());
				enemy->Target.ImpactDirection = (enemy->GetMesh()->GetCenterOfMass() - location);
				enemy->Target.ImpactDirection.Normalize();

				targets.Emplace(enemy->Target);
			}
		}
	}

	return res;
}

// -----------------------------------------------------------------------------------------
bool ACG_PlayerCharacter::GetTargetsInCone(ESpellCollisionType type, float radius, float angle, TArray<FCG_SpellTarget> & targets) const
{
	FVector origin = GetActorLocation();
	bool res = GetTargetsInSphere(type, radius, origin, targets);

	if (res)
	{
		for (uint32 i = targets.Num() - 1; i >= 0; --i)
		{
			FVector originToActor = targets[i].OwningActor->GetActorLocation() - origin;
			originToActor.Normalize();
			float dot = FVector::DotProduct(originToActor, GetActorForwardVector());
			if (FMath::Acos(dot) > angle)
			{
				targets.RemoveAt(i);
			}
		}
	}

	return res;
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::SpellFinishedCasting(UCG_SpellBase * spell)
{
	spell->OnFinishedCastingDelegate.RemoveAll(this);
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::ApplyDamage(int32 damage)
{
	Stats.Health = FMath::Clamp(Stats.Health - damage, 0, Stats.Health);

	if (Stats.Health <= 0)
	{
		OnDeath();
	}
	else
	{
		OnDamaged();
	}
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::ApplyStatus(uint8 status)
{
	SET_FLAG(Stats.Status, status);
	// TODO(RyanC): Need to figure out how statuses will work first.
}

// -----------------------------------------------------------------------------------------
void ACG_PlayerCharacter::ApplyForce(FVector direction, float strength)
{
	// TODO(RyanC): Ignoring for now until enemies are more complete
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
void ACG_PlayerCharacter::CastSpell()
{
	if (!IsCastingDisabled() && EquippedSpells.Num() > ActiveSpell)
	{
		if (!EquippedSpells[ActiveSpell]->IsSpellOnCooldown())
		{
			EquippedSpells[ActiveSpell]->OnBeginCast(this);
			EquippedSpells[ActiveSpell]->OnFinishedCastingDelegate.AddUObject(this, &ACG_PlayerCharacter::SpellFinishedCasting);
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

	CurrentState = EPlayerState::INSPECTING;

	InputComponent->BindAction("QuitInspection", IE_Pressed, this, &ACG_PlayerCharacter::InspectionEnded);
	InputComponent->BindAction("InspectionDrag", IE_Pressed, this, &ACG_PlayerCharacter::StartInspectionRotation);
	InputComponent->BindAction("InspectionDrag", IE_Released, this, &ACG_PlayerCharacter::StopInspectionRotation);

	SetMouseCursorShown(true);
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
