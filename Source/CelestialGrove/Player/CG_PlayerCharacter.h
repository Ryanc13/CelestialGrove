// ============================================================
// FILE: CG_PlayerCharacter.h
// AUTHOR: RyanC
// ============================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CG_GlobalDefines.h"
#include "UObject/NoExportTypes.h"
#include "CG_GlobalDefines.h"
#include "CG_PlayerCharacter.generated.h"

class UCameraComponent;
class USceneComponent;
class ACG_InteractableBase;
class UCG_SpellBase;

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
UENUM(BlueprintType)
enum class ESpellCollisionType : uint8
{
	ALL = 0,
	INANIMATE_ONLY,
	ANIMATE_ONLY
};

// ============================================================
UCLASS()
class CELESTIALGROVE_API ACG_PlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
// ============================================================
	ACG_PlayerCharacter();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float deltaTime) override;
	void BeginInspection(ACG_InteractableBase * const interactable);

	UFUNCTION(BlueprintCallable)
	bool TraceForCollision(FHitResult & result, float distance) const;

	UFUNCTION(BlueprintCallable)
	bool GetTargetsInSphere(ESpellCollisionType type, float radius, UPARAM(ref) FVector & location, TArray<FCG_SpellTarget> & targets) const;

	UFUNCTION(BlueprintCallable)
	bool GetTargetsInCone(ESpellCollisionType type, float radius, float angle, TArray<FCG_SpellTarget> & targets) const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	bool CreateProjectile(FVector direction, float speed, UCG_SpellBase * spell);

	UFUNCTION(BlueprintCallable)
	void SpellFinishedCasting(UCG_SpellBase * spell);

	UFUNCTION(BlueprintCallable)
	void ApplyDamage(int32 damage);

	UFUNCTION(BlueprintCallable)
	void ApplyStatus(uint8 status);

	UFUNCTION(BlueprintCallable)
	void ApplyForce(FVector direction, float strength);

	FORCEINLINE bool ShouldUpdatePlayer() const;

protected:
// ============================================================
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCursor(bool isDragging);

	UFUNCTION(BlueprintImplementableEvent)
	void FocusViewport();

	UFUNCTION(BlueprintImplementableEvent)
	void ChangeHUD(EPlayerState curState);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDamaged();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDeath();

// ============================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	EPlayerState CurrentState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	FCG_Stats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	FCG_SpellTarget Target;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	TArray<TObjectPtr<UCG_SpellBase>> EquippedSpells;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gameplay")
	float MovementSpeed;

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

private:
	// ============================================================
	// Input events
	void Interact();
	void CastSpell();
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
	FORCEINLINE bool IsCastingDisabled() const;

	// ============================================================
	FVector2D MouseDelta;
	uint32 isDraggingForInspection:1;
	uint8 ActiveSpell;
};

// ============================================================
// Inlined Functions
// -----------------------------------------------------------------------------------------
FORCEINLINE bool ACG_PlayerCharacter::ShouldUpdatePlayer() const
{
	return (CurrentState == EPlayerState::INSPECTING ||
			CurrentState == EPlayerState::PAUSED);
}
// -----------------------------------------------------------------------------------------
FORCEINLINE bool ACG_PlayerCharacter::IsMovementDisabled() const
{
	return (CurrentState == EPlayerState::INVENTORY ||
			CurrentState == EPlayerState::INSPECTING ||
			CurrentState == EPlayerState::PAUSED);
}
// -----------------------------------------------------------------------------------------
FORCEINLINE bool ACG_PlayerCharacter::IsMouseDisabled() const
{
	return (CurrentState == EPlayerState::INVENTORY ||
			CurrentState == EPlayerState::PAUSED);
}
// -----------------------------------------------------------------------------------------
FORCEINLINE bool ACG_PlayerCharacter::IsCastingDisabled() const
{
	return (CurrentState == EPlayerState::INSPECTING ||
			CurrentState == EPlayerState::INVENTORY ||
			CurrentState == EPlayerState::PAUSED);
}
// ============================================================
