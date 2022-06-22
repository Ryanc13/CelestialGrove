// ============================================================
// FILE: CG_10X_Unreal_Parse_Fixes.h
// AUTHOR: RyanC
// ============================================================

#if _FIX_UNREAL_PARSE_

// Unreal is doing some grimey c++ shit, fake defines only used by editor's parser so i can get autocomplete and goto def.

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
#define FMatrix UE::Math::TMatrix<float>
#define FRotationMatrix UE::Math::TRotationMatrix<float>

#endif // _FIX_UNREAL_PARSE_
