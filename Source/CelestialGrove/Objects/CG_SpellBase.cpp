// ============================================================
// FILE: CG_SpellBase.cpp
// AUTHOR: RyanC
// ============================================================

#include "Objects/CG_SpellBase.h"
#include "Misc/Guid.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "CG_PlayerCharacter.h"
#include "Sound/SoundCue.h"

// -----------------------------------------------------------------------------------------
UCG_SpellBase::UCG_SpellBase()
{
	CurrentSpellStep = ESpellComponentCategory::NONE;
}

// -----------------------------------------------------------------------------------------
void UCG_SpellBase::BuildSpell(TArray<FCG_SpellComponent> & components)
{
	check(components.Num() > 0);

	BaseCategory = components[0].Category;
	float cooldown = components[0].BaseCooldown;
	float strength = components[0].BaseStrength;

	for (FCG_SpellComponent component : components)
	{
		switch(component.Category)
		{
			case ESpellComponentCategory::TARGETING:
			{
				TargetingComponents.Emplace(component);
			}
			break;

			case ESpellComponentCategory::EFFECT:
			{
				EffectComponents.Emplace(component);
			}
			break;

			case ESpellComponentCategory::MODIFIERS:
			{
				ModifierComponents.Emplace(component);
			}
			break;

			default:
				checkNoEntry();
		}
		
		cooldown *= component.CooldownModifier;
		strength *= component.DamageModifier;
	}

	SpellStrength = (uint8)strength;
	SpellCooldown = cooldown;

	checkf(TargetingComponents.Num() > 1 && EffectComponents.Num() > 1, TEXT("Invalid spell! Needs 1 targeting component and 1 effect component"));

	BuildTargetingStyle();
}

// -----------------------------------------------------------------------------------------
void UCG_SpellBase::BuildTargetingStyle()
{
	if (TargetingComponents.Num() >= 3)
	{
		TargetingStyle = ETargetingStyles::CONE;
		return;
	}

	int32 targetField = 0;
	for (const FCG_SpellComponent & component : TargetingComponents)
	{
		SET_FLAG(targetField, 1 << (int32)component.Type);
	}

	if (COMPARE_FLAG(targetField, RADIUS_AT_POINT_FLAG))
	{
		TargetingStyle = ETargetingStyles::RADIUS_AT_POINT;
	}
	else if (COMPARE_FLAG(targetField, TARGET_SELF_FLAG))
	{
		TargetingStyle = ETargetingStyles::TARGET_SELF;
	}
	else if (COMPARE_FLAG(targetField, PROJECTILE_FLAG))
	{
		TargetingStyle = ETargetingStyles::PROJECTILE;
	}
	else if (COMPARE_FLAG(targetField, RADIUS_AT_ORIGIN_FLAG))
	{
		TargetingStyle = ETargetingStyles::RADIUS_AT_ORIGIN;
	}
	else if (COMPARE_FLAG(targetField, BEAM_FLAG))
	{
		TargetingStyle = ETargetingStyles::BEAM;
	}
	else if (COMPARE_FLAG(targetField, SELF_FORWARD_FLAG))
	{
		TargetingStyle = ETargetingStyles::SELF_FORWARD;
	}

	checkNoEntry();
}

// -----------------------------------------------------------------------------------------
void UCG_SpellBase::OnBeginCast(const ACG_PlayerCharacter * player)
{
	check(TargetingComponents.Num() > 0);
	check(EffectComponents.Num() > 0);

	CurrentSpellStep = ESpellComponentCategory::TARGETING;
	StartTargeting(player);
}

// -----------------------------------------------------------------------------------------
void UCG_SpellBase::OnFinishTargeting(const ACG_PlayerCharacter * player)
{
	// Early exit, all spell should have some target
	if (Targets.Num() == 0)
	{
		OnFinishEffect(player);
		return;
	}

	CurrentSpellStep = ESpellComponentCategory::EFFECT;
	StartEffect(player);
}

void UCG_SpellBase::OnFinishEffect(const ACG_PlayerCharacter * player)
{
	CurrentCooldown = SpellCooldown;
	CurrentSpellStep = ESpellComponentCategory::NONE;
	OnSpellComplete(player);
}

// -----------------------------------------------------------------------------------------
void UCG_SpellBase::AddTargetToSpell(UCG_SpellTargetStats * combatant)
{
	check(combatant);
	Targets.Emplace(combatant);
}

// -----------------------------------------------------------------------------------------
void UCG_SpellBase::UpdateSpell(float deltaTime, const ACG_PlayerCharacter * player)
{
	if (IsSpellOnCooldown())
	{
		CurrentCooldown = FMath::Clamp(CurrentCooldown-deltaTime, 0.f, CurrentCooldown);
	}

	switch (CurrentSpellStep)
	{
		case ESpellComponentCategory::TARGETING:
		{
			UpdateTargeting(deltaTime, player);
		}
		break;

		case ESpellComponentCategory::EFFECT:
		{
			UpdateEffect(deltaTime, player);
		}
		break;
	}
}

// -----------------------------------------------------------------------------------------
bool UCG_SpellBase::HasComponentsInCategory(ESpellComponentCategory category, TArray<ESpellComponentType> & types) const
{
	switch(category)
	{
		case ESpellComponentCategory::TARGETING:
		{
			for (ESpellComponentType type : types)
			{
				bool hasType = false;
				for (const FCG_SpellComponent & component : TargetingComponents)
				{
					if (component.Type == type)
					{
						hasType = true;
						break;
					}
				}

				if (!hasType) return false;
			}
		}
		break;

		case ESpellComponentCategory::EFFECT:
		{
			for (ESpellComponentType type : types)
			{
				bool hasType = false;
				for (const FCG_SpellComponent & component : EffectComponents)
				{
					if (component.Type == type)
					{
						hasType = true;
						break;
					}
				}

				if (!hasType) return false;
			}
		}
		break;

		case ESpellComponentCategory::MODIFIERS:
		{
			for (ESpellComponentType type : types)
			{
				bool hasType = false;
				for (const FCG_SpellComponent & component : ModifierComponents)
				{
					if (component.Type == type)
					{
						hasType = true;
						break;
					}
				}

				if (!hasType) return false;
			}
		}
		break;
	}

	return true;
}

const FCG_SpellComponent & UCG_SpellBase::GetBaseComponent() const
{
	switch (BaseCategory)
	{
		case ESpellComponentCategory::TARGETING:
		{
			return TargetingComponents[0];
		}
		break;

		case ESpellComponentCategory::MODIFIERS:
		{
			return ModifierComponents[0];
		}
		break;

		default:
			checkNoEntry();

		case ESpellComponentCategory::EFFECT:
		{
			return EffectComponents[0];
		}
		break;
	}
}
