// ============================================================
// FILE: CG_PlayerCharacter.h
// AUTHOR: RyanC
// ============================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CG_PlayerCharacter.generated.h"

class UCameraComponent;
class USceneComponent;
class ACG_InteractableBase;

// ============================================================
UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	DEFAULT = 0,
	INSPECTING,
	INVENTORY,
	PAUSED
};

// ============================================================
UCLASS()
class CELESTIALGROVE_API ACG_PlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACG_PlayerCharacter();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE bool ShouldUpdatePlayer() const;

	virtual void Tick(float deltaTime) override;

	void BeginInspection(ACG_InteractableBase * const interactable);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCursor(bool isDragging);

	UFUNCTION(BlueprintImplementableEvent)
	void FocusViewport();

	UFUNCTION(BlueprintImplementableEvent)
	void ChangeHUD(EPlayerState curState);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	EPlayerState CurrentState;

	// ============================================================
	// Inspection uproperties
	// The point which an inspectable object will interpolate to when inspection begins
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Inspection")
	TObjectPtr<USceneComponent> InspectionAnchorPoint;

	// Object we are actually inspecting
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay|Inspection")
	TSoftObjectPtr<ACG_InteractableBase> InspectionTarget;

	// Maximum distance interactions are triggered from
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Inspection")
	float InteractionDistance;

	// How fast the inspectable object is moved to the attachment point
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Inspection")
	float InspectionInterpSpeed;

	// How fast the object will rotate about the desired axis during inspection (degrees)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Inspection")
	FVector2D InspectionRotationSpeed;

	// This determines how large the difference between the normalized mouse deltas need to
	// before it will rotate about both axis at once
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay|Inspection")
	float InspectionAxisTolerance;
	// ===========================================================
	// Misc uproperties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay", Meta = (ClampMin="0", ClampMax="1"))
	FVector2D MouseSensitivity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float ThrowStrength;

	// Non-UPROPERTY member variables
	// ============================================================
	FVector2D MouseDelta;
	uint32 isDraggingForInspection:1;

private:
	// ============================================================
	// Input events
	void Interact();
	void QuitInspection();
	void StartInspectionRotation();
	void StopInspectionRotation();

	void MoveForward(float value);
	void MoveRight(float value);

	void Turn(float rate);
	void LookUp(float rate);
	// ============================================================

	void InspectionEnded();
	void SetMouseCursorShown(bool isShown);

	FORCEINLINE bool IsMovementDisabled() const;
	FORCEINLINE bool IsMouseDisabled() const;
};

// ============================================================
// Inlined Functions
FORCEINLINE bool ACG_PlayerCharacter::IsMovementDisabled() const
{
	return (CurrentState == EPlayerState::INVENTORY ||
			CurrentState == EPlayerState::INSPECTING ||
			CurrentState == EPlayerState::PAUSED);
}

FORCEINLINE bool ACG_PlayerCharacter::IsMouseDisabled() const
{
	return (CurrentState == EPlayerState::INVENTORY ||
			CurrentState == EPlayerState::PAUSED);
}

FORCEINLINE bool ACG_PlayerCharacter::ShouldUpdatePlayer() const
{
	return (CurrentState == EPlayerState::INSPECTING ||
			CurrentState == EPlayerState::PAUSED);
}
// ============================================================
