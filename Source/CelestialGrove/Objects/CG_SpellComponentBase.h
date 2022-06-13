// ============================================================
// FILE: CG_SpellComponentBase.h
// AUTHOR: RyanC
// ============================================================
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CG_SpellComponentBase.generated.h"

// ============================================================
// Components Tags are used to determine the priority in which
// the component modifies the overall spell
UENUM(BlueprintType)
enum class EComponentTags : uint8
{
	TARGETING = 0,
	TARGET_MODIFIER,
	EFFECT,
	EFFECT_MODIFIER
};

// ============================================================
UCLASS()
class CELESTIALGROVE_API UCG_SpellComponentBase : public UObject
{
	GENERATED_BODY()

// ============================================================
public:


// ============================================================
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Gameplay)
	EComponentTags Tag;
};
