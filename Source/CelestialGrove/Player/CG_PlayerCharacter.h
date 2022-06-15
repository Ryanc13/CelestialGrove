// ============================================================
// FILE: CG_PlayerCharacter.h
// AUTHOR: RyanC
// ============================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CG_GlobalDefines.h"
#include "UObject/NoExportTypes.h"
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
UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECombatStatuses : uint8
{
	NONE = 0x00,
	HELD = 0x01,
	ON_FIRE = 0x02,
	STUNNED = 0x04,
	DEATH = 0x08
};

ENUM_CLASS_FLAGS(ECombatStatuses);

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
class CELESTIALGROVE_API UCG_SpellTargetStats : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "ECombatStatuses"))
	uint8 Status;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<AActor> owningActor;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasStatus(UPARAM(Meta = (Bitmask, BitmaskEnum = ECombatStatuses)) uint8 flag);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetStatus(UPARAM(Meta = (Bitmask, BitmaskEnum = ECombatStatuses)) uint8 flag);
};

// ============================================================
// FTargetPair Inlines
// -----------------------------------------------------------------------------------------
FORCEINLINE bool UCG_SpellTargetStats::HasStatus(uint8 flag)
{
	if (flag == 0)
	{
		return (Status == 0);
	}

	return COMPARE_FLAG(Status, flag);
}
// -----------------------------------------------------------------------------------------
FORCEINLINE void UCG_SpellTargetStats::SetStatus(uint8 flag)
{
	SET_FLAG(Status, flag);
}
// ============================================================

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
	bool TryCastingSpell(uint8 spellIndex);

	UFUNCTION(BlueprintCallable)
	bool TraceForCollision(FHitResult & result, float distance) const;

	UFUNCTION(BlueprintCallable)
	bool GetTargetsInSphere(ESpellCollisionType type, float radius, UPARAM(ref) FVector & location, TArray<UCG_SpellTargetStats*> & targets) const;

	UFUNCTION(BlueprintCallable)
	bool GetTargetsInCone(ESpellCollisionType type, float radius, float angle, TArray<UCG_SpellTargetStats*> & targets) const;

	FORCEINLINE bool ShouldUpdatePlayer() const;
	FORCEINLINE UCG_SpellTargetStats * GetStats() const;

protected:
// ============================================================
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCursor(bool isDragging);

	UFUNCTION(BlueprintImplementableEvent)
	void FocusViewport();

	UFUNCTION(BlueprintImplementableEvent)
	void ChangeHUD(EPlayerState curState);

// ============================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	EPlayerState CurrentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	TObjectPtr<UCG_SpellTargetStats> Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	TArray<TObjectPtr<UCG_SpellBase>> EquippedSpells;

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

	// ============================================================
	TArray<uint8> SpellsBeingCast;
};

// ============================================================
// Inlined Functions
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
FORCEINLINE bool ACG_PlayerCharacter::ShouldUpdatePlayer() const
{
	return (CurrentState == EPlayerState::INSPECTING ||
			CurrentState == EPlayerState::PAUSED);
}
// -----------------------------------------------------------------------------------------
FORCEINLINE UCG_SpellTargetStats * ACG_PlayerCharacter::GetStats() const
{
	return Stats;
}
// ============================================================
