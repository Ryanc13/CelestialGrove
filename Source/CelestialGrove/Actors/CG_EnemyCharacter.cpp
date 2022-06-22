// ============================================================
// FILE: CG_EnemyCharacter.cpp
// AUTHOR: RyanC
// ============================================================

#include "CG_EnemyCharacter.h"
#include "CG_PlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"

// -----------------------------------------------------------------------------------------
ACG_EnemyCharacter::ACG_EnemyCharacter()
{
	// ============================================================
	// Tick settings
	// TODO(RyanC): Disable tick for all enemies until they are within an acceptable distance of the player.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// ============================================================
	// Component Initialization
	Health = CreateDefaultSubobject<UWidgetComponent>(TEXT("Health"));
	Health->SetupAttachment(RootComponent);

	RagdollSettleTolerance = 0.5f;
	PushByForceResistance = 150.f;
	IsInRagdoll = false;
}

// -----------------------------------------------------------------------------------------
void ACG_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	Target.OwningActor = this;
	Target.ApplyDamageDelegate.AddUObject(this, &ACG_EnemyCharacter::ApplyDamage);
	Target.ApplyStatusDelegate.AddUObject(this, &ACG_EnemyCharacter::ApplyStatus);
	Target.ApplyForceDelegate.AddUObject(this, &ACG_EnemyCharacter::ApplyForce);
}

// -----------------------------------------------------------------------------------------
void ACG_EnemyCharacter::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (IsInRagdoll)
	{
		// ============================================================
		// Keep moving the root to the ragdolling mesh so the health bar stays with it
		FVector socketLocation = GetMesh()->GetSocketLocation(RagdollSocketToFollow);
		RootComponent->SetWorldLocation(socketLocation + CapsuleToMeshOffset);

		// ============================================================
		// Now we check to see if we have already settled using the defined tolerance
		FVector currentPosition = GetMesh()->GetComponentLocation();
		FVector positionDelta = currentPosition - PreviousMeshPosition;
		float length = positionDelta.Length();
		if (length < RagdollSettleTolerance)
		{
			IsInRagdoll = false;
			// We are now settled turn off simulation and call into blueprints to start get up animation
			GetMesh()->SetSimulatePhysics(false);
			GetMesh()->PutAllRigidBodiesToSleep();

			OnBeginStandUp();
		}

		PreviousMeshPosition = GetMesh()->GetComponentLocation();
	}
}

// -----------------------------------------------------------------------------------------
void ACG_EnemyCharacter::ApplyDamage(int32 damage)
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
void ACG_EnemyCharacter::ApplyStatus(uint8 status)
{
	SET_FLAG(Stats.Status, status);

	// TODO(RyanC): Need to figure out how statuses will work first.
}

// -----------------------------------------------------------------------------------------
void ACG_EnemyCharacter::ApplyForce(FVector direction, float strength)
{
	if (PushByForceResistance <= strength)
	{
		IsInRagdoll = true;
		PreviousMeshPosition = FVector::ZeroVector;

		// Detach mesh to prepare for force being applied
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();

		CapsuleToMeshOffset = GetCapsuleComponent()->GetComponentLocation() - GetMesh()->GetComponentLocation();

		// Apply in blueprints in case more set up is needed for the specific enemy.
		OnApplyForce(direction * strength * GetMesh()->GetMass());
	}
}

// -----------------------------------------------------------------------------------------
void ACG_EnemyCharacter::OnApplyForce_Implementation(FVector toApply)
{
	GetMesh()->AddImpulse(toApply);
}

// -----------------------------------------------------------------------------------------
void ACG_EnemyCharacter::OnFinishedStandingUp()
{
	FVector socketLocation = GetMesh()->GetSocketLocation(RagdollSocketToFollow);
	RootComponent->SetWorldLocation(socketLocation + CapsuleToMeshOffset);

	// Only care about the rotation around the up axis
	FRotator newRotation = RootComponent->GetComponentRotation();
	newRotation.Yaw = GetMesh()->GetComponentRotation().Yaw;
	RootComponent->SetWorldRotation(newRotation);

	// NOTE(RyanC): Hopefully this is redundant but if we still have popping issues then may need to interpolate.
	GetMesh()->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	GetMesh()->SetRelativeLocation(-CapsuleToMeshOffset);
	GetMesh()->SetRelativeRotation(FRotator::ZeroRotator);
}
