// ============================================================
// FILE: CG_InteractableBase.h
// AUTHOR: RyanC
// ============================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
	// Sets default values for this actor's properties
	ACG_InteractableBase();

	UFUNCTION(BlueprintCallable)
	void OnInteracted(ACG_PlayerCharacter * player);

	FVector InspectionOffset() const;

	UFUNCTION(BlueprintNativeEvent)
	void OnEndInspection(FVector throwVector, float throwStrength, bool shouldThrow = true);
	void OnEndInspection_Implementation(FVector throwVector, float throwStrength, bool shouldThrow = true);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void Interact(ACG_PlayerCharacter * player);

	UFUNCTION(BlueprintImplementableEvent)
	void Loot(ACG_PlayerCharacter * player);

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

};
