// ============================================================
// FILE: CG_InteractableBase.cpp
// AUTHOR: RyanC
// ============================================================

#include "CG_InteractableBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"
#include "CG_PlayerCharacter.h"
#include "CG_SpellBase.h"

// -----------------------------------------------------------------------------------------
ACG_InteractableBase::ACG_InteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	StaticMesh->SetSimulatePhysics(true);
	RootComponent = StaticMesh;

	InspectionCenter = CreateDefaultSubobject<USceneComponent>(TEXT("Inspection Center"));
	InspectionCenter->SetupAttachment(StaticMesh);
}

// -----------------------------------------------------------------------------------------
void ACG_InteractableBase::BeginPlay()
{
	Super::BeginPlay();

	Target.OwningActor = this;
	Target.ApplyDamageDelegate.AddUObject(this, &ACG_InteractableBase::ApplyDamage);
	Target.ApplyStatusDelegate.AddUObject(this, &ACG_InteractableBase::ApplyStatus);
	Target.ApplyForceDelegate.AddUObject(this, &ACG_InteractableBase::ApplyForce);
}

// -----------------------------------------------------------------------------------------
void ACG_InteractableBase::OnInteracted(ACG_PlayerCharacter * player)
{
	UE_LOG(LogCelestialGrove, Log, TEXT("Interactable %s has been activated."), *GetFName().ToString());
	if (Interactions == (uint8)EInteractableFlags::UNINTERACTABLE)
	{
		return; // NOTE(RyanC): This should probably just not happen, may change to an assert later
	}

	// We first attempt to trigger a basic interaction with the object
	if (COMPARE_FLAG(Interactions, (uint8)EInteractableFlags::INTERACTABLE))
	{
		UE_LOG(LogCelestialGrove, Log, TEXT("Interactable %s began interaction"), *GetFName().ToString());
		Interact(player);
	}
	// If no interactions are left then we check if the object can be inspected
	else if (COMPARE_FLAG(Interactions, (uint8)EInteractableFlags::INSPECTABLE))
	{
		UE_LOG(LogCelestialGrove, Log, TEXT("Interactable %s began inspection"), *GetFName().ToString());
		OnBeginInspection(player);
	}
	// Lastly we loot the object
	else if (COMPARE_FLAG(Interactions, (uint8)EInteractableFlags::LOOTABLE))
	{
		UE_LOG(LogCelestialGrove, Log, TEXT("Interactable %s began looting"), *GetFName().ToString());
		Loot(player);
	}
}

// -----------------------------------------------------------------------------------------
FVector ACG_InteractableBase::InspectionOffset() const
{
	return (GetActorLocation() - InspectionCenter->GetComponentLocation());
}

// -----------------------------------------------------------------------------------------
FVector ACG_InteractableBase::GetCenterOfMass() const
{
	return StaticMesh->GetCenterOfMass();
}

// -----------------------------------------------------------------------------------------
void ACG_InteractableBase::OnBeginInspection_Implementation(ACG_PlayerCharacter * player)
{
	player->BeginInspection(this);
	StaticMesh->SetEnableGravity(false);
	StaticMesh->SetSimulatePhysics(false);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// -----------------------------------------------------------------------------------------
void ACG_InteractableBase::OnEndInspection_Implementation(FVector throwVector, float throwStrength, bool shouldThrow)
{
	StaticMesh->SetEnableGravity(true);
	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	if (shouldThrow)
	{
		ApplyForce(throwVector, throwStrength);
	}
}

// -----------------------------------------------------------------------------------------
void ACG_InteractableBase::ApplyDamage(int32 damage)
{
	Stats.Health = FMath::Clamp(Stats.Health - damage, 0, Stats.Health);

	if (Stats.Health == 0)
	{
		OnDestroyed();
	}
	else
	{
		OnDamaged();
	}
}

// -----------------------------------------------------------------------------------------
void ACG_InteractableBase::ApplyForce(FVector direction, float strength)
{
	// NOTE(RyanC): Probably will end up only turning on physics when a force happens and then disable when settled.
	// for now its just on by default.
	StaticMesh->AddForce(direction * strength * StaticMesh->GetMass());
}
