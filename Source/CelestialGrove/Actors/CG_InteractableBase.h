// ============================================================
// FILE: CG_InteractableBase.h
// AUTHOR: RyanC
// ============================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CG_GlobalDefines.h"
#include "CG_InteractableBase.generated.h"

class UStaticMeshComponent;
class ACG_PlayerCharacter;
class APlayerController;

// ============================================================
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EInteractableFlags : uint8
{
	INTERACTABLE = 0x01,
	INSPECTABLE = 0x02,
	LOOTABLE = 0x04,

	UNINTERACTABLE = 0x00
};

ENUM_CLASS_FLAGS(EInteractableFlags);

// ============================================================
UCLASS()
class CELESTIALGROVE_API ACG_InteractableBase : public AActor
{
	GENERATED_BODY()
	
public:
// ============================================================
	ACG_InteractableBase();
	
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void OnInteracted(ACG_PlayerCharacter * player);

	FVector InspectionOffset() const;
	FVector GetCenterOfMass() const;

	UFUNCTION(BlueprintNativeEvent)
	void OnEndInspection(FVector throwVector, float throwStrength, bool shouldThrow = true);
	void OnEndInspection_Implementation(FVector throwVector, float throwStrength, bool shouldThrow = true);

// ===========================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	FCG_Stats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	FCG_SpellTarget Target;

protected:
// ============================================================
	UFUNCTION(BlueprintImplementableEvent)
	void Interact(ACG_PlayerCharacter * player);

	UFUNCTION(BlueprintImplementableEvent)
	void Loot(ACG_PlayerCharacter * player);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDamaged();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDestroyed();

	UFUNCTION(BlueprintImplementableEvent)
	void ApplyStatus(uint8 status); // NOTE(RyanC): not sure of the use of this yet but implementing it just in case

	UFUNCTION(BlueprintNativeEvent)
	void OnBeginInspection(ACG_PlayerCharacter * player);
	void OnBeginInspection_Implementation(ACG_PlayerCharacter * player);

// ============================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
	TObjectPtr<USceneComponent> InspectionCenter;

	UPROPERTY(EditAnywhere, Meta = (Bitmask, BitmaskEnum = "EInteractableFlags"))
	uint8 Interactions;

private:
// ============================================================
	void ApplyDamage(int32 damage);
	void ApplyForce(FVector direction, float strength);
};
