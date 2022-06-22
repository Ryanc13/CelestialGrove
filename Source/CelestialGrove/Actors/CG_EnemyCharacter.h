// ============================================================
// FILE: CG_EnemyCharacter.h
// AUTHOR: RyanC
// ============================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CG_GlobalDefines.h"
#include "CG_EnemyCharacter.generated.h"

class UWidgetComponent;

UCLASS(Blueprintable)
class CELESTIALGROVE_API ACG_EnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
// ============================================================
	ACG_EnemyCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float deltaTime) override;

	UFUNCTION(BlueprintCallable)
	void ApplyDamage(int32 damage);

	UFUNCTION(BlueprintCallable)
	void ApplyStatus(uint8 status);

	UFUNCTION(BlueprintCallable)
	void ApplyForce(FVector direction, float strength);

// ============================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	FCG_Stats Stats;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	FCG_SpellTarget Target;

protected:
// ============================================================
	UFUNCTION(BlueprintImplementableEvent)
	void OnDamaged();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDeath();

	// Native event if you choose to override this then you have to apply the force yourself or call to parent.
	UFUNCTION(BlueprintNativeEvent)
	void OnApplyForce(FVector toApply);
	void OnApplyForce_Implementation(FVector toApply);

	UFUNCTION(BlueprintImplementableEvent)
	void OnBeginStandUp();

	UFUNCTION(BlueprintCallable)
	void OnFinishedStandingUp();

// ============================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	TObjectPtr<UWidgetComponent> Health;

	// If the force being applied is greater than this value then the enemy will ragdoll.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Gameplay)
	float PushByForceResistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	float RagdollSettleTolerance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	FName RagdollSocketToFollow;

	FVector CapsuleToMeshOffset;
	FVector PreviousMeshPosition;
	uint32 IsInRagdoll:1;
};
