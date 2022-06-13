#pragma once

// ============================================================
// Global defines for Celestial Grove
// ============================================================
#define COMPARE_FLAG(field, flag) ((field & flag) == flag)
#define TOGGLE_FLAG(field, flag) (field = field ^ flag)
#define SET_FLAG(field, flag) (field = field | flag)
#define CLEAR_FLAG(field, flag) (field = field & ~flag)

#define INTERACTABLE_COLLISION_CHANNEL ECollisionChannel::ECC_GameTraceChannel1

#define internal static
#define global static

DECLARE_LOG_CATEGORY_EXTERN(LogCelestialGrove, Log, All);
