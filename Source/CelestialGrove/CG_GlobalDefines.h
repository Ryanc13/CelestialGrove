#pragma once

#include "CoreMinimal.h"
#include "CG_GlobalDefines.generated.h"

// ============================================================
// Global defines for Celestial Grove
// ============================================================
#define COMPARE_FLAG(field, flag) ((field & flag) == flag)
#define TOGGLE_FLAG(field, flag) (field = field ^ flag)
#define SET_FLAG(field, flag) (field = field | flag)
#define CLEAR_FLAG(field, flag) (field = field & ~flag)

#define INTERACTABLE_COLLISION_CHANNEL ECollisionChannel::ECC_GameTraceChannel1
#define SPELL_TRACE_CHANNEL ECollisionChannel::ECC_GameTraceChannel2
#define INANIMATE_COLLISION_CHANNEL ECollisionChannel::ECC_GameTraceChannel3
#define ANIMATE_COLLISION_CHANNEL ECollisionChannel::ECC_GameTraceChannel4

#define internal static
#define global static

DECLARE_LOG_CATEGORY_EXTERN(LogCelestialGrove, Log, All);

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

DECLARE_MULTICAST_DELEGATE_OneParam(FApplyDamageSignature, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FApplyStatusSignature, uint8);
DECLARE_MULTICAST_DELEGATE_TwoParams(FApplyForceSignature, FVector, float);

// ============================================================
USTRUCT(BlueprintType)
struct CELESTIALGROVE_API FCG_SpellTarget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<AActor> OwningActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector ImpactDirection;

	FApplyDamageSignature ApplyDamageDelegate;
	FApplyStatusSignature ApplyStatusDelegate;
	FApplyForceSignature ApplyForceDelegate;
};

// ============================================================
USTRUCT(BlueprintType)
struct CELESTIALGROVE_API FCG_Stats
{
	GENERATED_BODY()

public:
	// ============================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Health = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "ECombatStatuses"))
	uint8 Status = 0;
};
