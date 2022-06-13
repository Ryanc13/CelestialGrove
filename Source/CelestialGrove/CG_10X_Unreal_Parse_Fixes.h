// ============================================================
// FILE: CG_10X_Unreal_Parse_Fixes.h
// AUTHOR: RyanC
// ============================================================

#ifndef CG_10X_UNREAL_PARSE_FIXES_H
#define CG_10X_UNREAL_PARSE_FIXES_H

#if _FIX_UNREAL_PARSE_

#include "Math/Vector.h"
#include "Math/Vector2D.h"
#include "Math/TransformVectorized.h"
#include "Math/Rotator.h"
#include "Math/Quat.h"

#define FVector UE::Math::TVector<float>
#define FTransform UE::Math::TTransform<float>
#define FVector2f UE::Math::TVector2<float>
#define FVector2D UE::Math::TVector<float, 2>
#define FRotator UE::Math::TRotator<double>
#define FQuat UE::Math::TQuat<double>

#endif // _FIX_UNREAL_PARSE_

#endif // CG_10X_UNREAL_PARSE_FIXES_H
