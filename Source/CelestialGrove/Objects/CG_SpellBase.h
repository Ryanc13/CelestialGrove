// ============================================================
// FILE: CG_SpellBase.h
// AUTHOR: RyanC
// ============================================================

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/NoExportTypes.h"
#include "CG_GlobalDefines.h"
#include "CG_SpellBase.generated.h"

class AActor;
class UNiagaraSystem;
class USoundCue;
struct FGuid;

// ============================================================
UENUM(BlueprintType)
enum class ESpellComponentCategory : uint8
{
	NONE = 0, // NOTE(RyanC): This doesn't mean anything but going to use it for the spell's state machine
	TARGETING,
	EFFECT,
	MODIFIERS
};

// ============================================================
UENUM(BlueprintType)
enum class ESpellComponentType : uint8
{
	// ============================================================
	// Targeting Components
	RADIUS_TARGETING = 0,
	SELF_TARGETING,
	FOREWARD_TARGETING,

	// ============================================================
	// Effect Components
	FIRE_EFFECT,
	TELEKINETIC_EFFECT,
	ELECTRIC_EFFECT,

	// ============================================================
	// Modifier Components
	AMPLIFY_MODIFIER,
	INANIMATE_MODIFIER,
	QUICK_MODIFIER,
	CONTINUOUS_MODIFIER
};

#define RADIUS_AT_POINT_FLAG (1<<(int32)ESpellComponentType::RADIUS_TARGETING)
#define TARGET_SELF_FLAG (1<<(int32)ESpellComponentType::SELF_TARGETING)
#define PROJECTILE_FLAG (1<<(int32)ESpellComponentType::FOREWARD_TARGETING)
#define RADIUS_AT_ORIGIN_FLAG ((1<<(int32)ESpellComponentType::RADIUS_TARGETING) | (1<<(int32)ESpellComponentType::SELF_TARGETING))
#define BEAM_FLAG ((1<<(int32)ESpellComponentType::RADIUS_TARGETING) | (1<<(int32)ESpellComponentType::FOREWARD_TARGETING))
#define SELF_FORWARD_FLAG ((1<<(int32)ESpellComponentType::FOREWARD_TARGETING) | (1<<(int32)ESpellComponentType::SELF_TARGETING))

// ============================================================
UENUM(BlueprintType)
enum class ETargetingStyles : uint8
{
	RADIUS_AT_POINT = 0,
	TARGET_SELF,
	PROJECTILE,
	RADIUS_AT_ORIGIN,
	BEAM,
	SELF_FORWARD,
	CONE
};

// ============================================================
DECLARE_MULTICAST_DELEGATE_OneParam(FSpellFinishedCastingSignature, UCG_SpellBase *);

// ============================================================
USTRUCT(BlueprintType)
struct CELESTIALGROVE_API FCG_SpellComponent : public FTableRowBase
{
	GENERATED_BODY()
	
public:
// ============================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGuid GUID = FGuid::NewGuid();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESpellComponentCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESpellComponentType Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseEffectStrength = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseTargetStrenth = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseCooldown = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Phonetic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture> Symbol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> BaseEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundCue> BaseSfx;
};

// ============================================================
UCLASS(Blueprintable)
class CELESTIALGROVE_API UCG_SpellBase : public UObject
{
	GENERATED_BODY()

public:
// ============================================================
	UCG_SpellBase();

	UFUNCTION(BlueprintCallable)
	void BuildSpell(UPARAM(ref) TArray<FCG_SpellComponent> & components);

	UFUNCTION(BlueprintCallable)
	void OnBeginCast(const ACG_PlayerCharacter * player);

	UFUNCTION(BlueprintCallable)
	void OnFinishTargeting(const ACG_PlayerCharacter * player);
	
	UFUNCTION(BlueprintCallable)
	void OnFinishEffect(const ACG_PlayerCharacter * player);

	UFUNCTION(BlueprintCallable)
	void AddTargetToSpell(UPARAM(ref) FCG_SpellTarget & target);

	UFUNCTION(BlueprintCallable)
	void AddTargetToSpellWithImpactDir(UPARAM(ref) FCG_SpellTarget & target, FVector direction);

	UFUNCTION(BlueprintCallable)
	void UpdateSpell(float deltaTime, const ACG_PlayerCharacter * player);

	UFUNCTION(BlueprintCallable)
	bool HasComponentsInCategory(ESpellComponentCategory category, UPARAM(ref) TArray<ESpellComponentType> & types) const;

	UFUNCTION(BlueprintCallable)
	const FCG_SpellComponent & GetBaseComponent() const;

	UFUNCTION(BlueprintCallable)
	void ApplyDamageToTargets(int32 finalDamage) const;

	UFUNCTION(BlueprintCallable)
	void ApplyStatusToTargets(ECombatStatuses newStatus) const;

	UFUNCTION(BlueprintCallable)
	void ApplyForceToTargets(float strength) const;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetSpellEffectStrength() const;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetSpellTargetStrength() const;
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsSpellOnCooldown() const;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool ShouldSpellCooldown() const;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE ESpellComponentType GetEffectBase() const;

// ============================================================
	FSpellFinishedCastingSignature OnFinishedCastingDelegate;

protected:
// ============================================================
	UFUNCTION(BlueprintImplementableEvent)
	void StartTargeting(const ACG_PlayerCharacter * player);

	UFUNCTION(BlueprintImplementableEvent)
	void StartEffect(const ACG_PlayerCharacter * player);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateTargeting(float deltaTime, const ACG_PlayerCharacter * player);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateEffect(float deltaTime, const ACG_PlayerCharacter * player);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSpellComplete(const ACG_PlayerCharacter * player);

// ===========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	TArray<FCG_SpellComponent> TargetingComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	TArray<FCG_SpellComponent> EffectComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	TArray<FCG_SpellComponent> ModifierComponents;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gameplay)
	ETargetingStyles TargetingStyle;

private:
// ============================================================
	void BuildTargetingStyle();

// ============================================================
	ESpellComponentCategory CurrentSpellStep;
	ESpellComponentCategory BaseCategory;

	int32 SpellEffectStrength;
	float SpellTargetStrength;
	float SpellCooldown;
	float CurrentCooldown;

	TArray<FCG_SpellTarget> Targets;
};

// ============================================================
// INLINED FUNCTIONS
// -----------------------------------------------------------------------------------------
FORCEINLINE int32 UCG_SpellBase::GetSpellEffectStrength() const
{
	return SpellEffectStrength;
}
// -----------------------------------------------------------------------------------------
FORCEINLINE float UCG_SpellBase::GetSpellTargetStrength() const
{
	return SpellTargetStrength;
}
// -----------------------------------------------------------------------------------------
FORCEINLINE bool UCG_SpellBase::IsSpellOnCooldown() const
{
	return (CurrentCooldown > 0.0f);
}
// -----------------------------------------------------------------------------------------
FORCEINLINE bool UCG_SpellBase::ShouldSpellCooldown() const
{
	return (IsSpellOnCooldown() && CurrentSpellStep == ESpellComponentCategory::NONE);
}
// -----------------------------------------------------------------------------------------
FORCEINLINE ESpellComponentType UCG_SpellBase::GetEffectBase() const
{
	check(EffectComponents.Num() > 0);
	return EffectComponents[0].Type;
}
// ============================================================
