#pragma once
#include "BaseDefine.h"
#include "BaseStruct.h"

const Float4 g_Select1110 = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000 };

inline bool InRectangle(float pointX, float pointY, float posX, float posY, float sizeX, float sizeY)
{
    return posX <= pointX && pointX <= posX + sizeX && posY <= pointY && pointY <= posY + sizeY;
}

inline Float4 VectorDot(Float4 v1, Float4 v2)
{
    float fValue = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    return Float4(fValue, fValue, fValue, fValue);
}

inline Float4 VectorSqrt(Float4 v)
{
    return Float4(sqrtf(v.x), sqrtf(v.y), sqrtf(v.z), sqrtf(v.z));
}

inline Float4 VectorLengthSqrt(Float4 v)
{
    return VectorDot(v, v);
}

inline Float4 VectorLength(const Float4& v)
{
    Float4 Result;
    Result = VectorLengthSqrt(v);
    Result = VectorSqrt(Result);
    return Result;
}

inline Float4 VectorNormalize(Float4 v)
{
    float fLength;
    Float4 vResult;

    vResult = VectorLength(v);
    fLength = vResult.x;

    if (fLength > 0)
    {
        fLength = 1.0f / fLength;
    }

    vResult.x = v.x * fLength;
    vResult.y = v.y * fLength;
    vResult.z = v.z * fLength;
    vResult.w = v.w * fLength;
    return vResult;
}

inline Float4 VectorCross(Float4 v1, Float4 v2)
{
    Float4 vResult = {
            (v1[1] * v2[2]) - (v1[2] * v2[1]),
            (v1[2] * v2[0]) - (v1[0] * v2[2]),
            (v1[0] * v2[1]) - (v1[1] * v2[0]),
            0.0f
    };
    return vResult;
}

inline Float4 VectorNegate(Float4 v) noexcept
{
    return { -v[0], -v[1], -v[2], -v[3] };
}

inline Float4 VectorSelect(Float4 v1, Float4 v2, Float4 control)
{
    return {
            (v1.GetUintIndex(0) & ~control.GetUintIndex(0)) | (v2.GetUintIndex(0) & control.GetUintIndex(0)),
            (v1.GetUintIndex(1) & ~control.GetUintIndex(1)) | (v2.GetUintIndex(1) & control.GetUintIndex(1)),
            (v1.GetUintIndex(2) & ~control.GetUintIndex(2)) | (v2.GetUintIndex(2) & control.GetUintIndex(2)),
            (v1.GetUintIndex(3) & ~control.GetUintIndex(3)) | (v2.GetUintIndex(3) & control.GetUintIndex(3)),
    };
}

inline Float4 VectorMergeXY(const Float4& v1, const Float4& v2)
{
    return { v1[0], v2[0], v1[1], v2[1] };
}

inline Float4 VectorMergeZW(const Float4& v1, const Float4& v2)
{
    return { v1[2], v2[2], v1[3], v2[3] };
}

inline Matrix4x4 Matrix4x4_Transpose(const Matrix4x4& mat)
{
    Matrix4x4 P;
    P.GetFloat4(0) = VectorMergeXY(mat.GetFloat4(0), mat.GetFloat4(2)); // m00m20m01m21
    P.GetFloat4(1) = VectorMergeXY(mat.GetFloat4(1), mat.GetFloat4(3)); // m10m30m11m31
    P.GetFloat4(2) = VectorMergeZW(mat.GetFloat4(0), mat.GetFloat4(2)); // m02m22m03m23
    P.GetFloat4(3) = VectorMergeZW(mat.GetFloat4(1), mat.GetFloat4(3)); // m12m32m13m33

    Matrix4x4 MT;
    MT.GetFloat4(0) = VectorMergeXY(P.GetFloat4(0), P.GetFloat4(1)); // m00m10m20m30
    MT.GetFloat4(1) = VectorMergeZW(P.GetFloat4(0), P.GetFloat4(1)); // m01m11m21m31
    MT.GetFloat4(2) = VectorMergeXY(P.GetFloat4(2), P.GetFloat4(3)); // m02m12m22m32
    MT.GetFloat4(3) = VectorMergeZW(P.GetFloat4(2), P.GetFloat4(3)); // m03m13m23m33

    return MT;
}

inline void Matrix4x4_ScalarSinCos(float* sin, float* cos, float angle)
{
    // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
    float quotient = 1 / MATH_2PI * angle;
    if (angle >= 0.0f)
    {
        quotient = static_cast<float>(static_cast<int>(quotient + 0.5f));
    }
    else
    {
        quotient = static_cast<float>(static_cast<int>(quotient - 0.5f));
    }
    float y = angle - MATH_2PI * quotient;

    // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
    float sign;
    if (y > MATH_2PI / 2)
    {
        y = MATH_PI - y;
        sign = -1.0f;
    }
    else if (y < -MATH_2PI / 2)
    {
        y = -MATH_PI - y;
        sign = -1.0f;
    }
    else
    {
        sign = +1.0f;
    }

    float y2 = y * y;

    // 11-degree minimax approximation
    *sin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

    // 10-degree minimax approximation
    float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
    *cos = sign * p;
}

inline Matrix4x4 Matrix4x4_RotationY(float angle)
{
	float    fSinAngle;
	float    fCosAngle;
	Matrix4x4_ScalarSinCos(&fSinAngle, &fCosAngle, angle);

	Matrix4x4 M;
	M.m[0][0] = fCosAngle;
	M.m[0][1] = 0.0f;
	M.m[0][2] = -fSinAngle;
	M.m[0][3] = 0.0f;

	M.m[1][0] = 0.0f;
	M.m[1][1] = 1.0f;
	M.m[1][2] = 0.0f;
	M.m[1][3] = 0.0f;

	M.m[2][0] = fSinAngle;
	M.m[2][1] = 0.0f;
	M.m[2][2] = fCosAngle;
	M.m[2][3] = 0.0f;

	M.m[3][0] = 0.0f;
	M.m[3][1] = 0.0f;
	M.m[3][2] = 0.0f;
	M.m[3][3] = 1.0f;
	return M;
}

inline Float4 VectorSplatX(const Float4& v) noexcept
{
    Float4 vResult;
    vResult[0] = vResult[1] = vResult[2] = vResult[3] = v[0];
    return vResult;
}

inline Float4 VectorSplatY(const Float4& v) noexcept
{
    Float4 vResult;
    vResult[0] = vResult[1] = vResult[2] = vResult[3] = v[1];
    return vResult;
}

inline Float4 VectorSplatZ(const Float4& v) noexcept
{
    Float4 vResult;
    vResult[0] = vResult[1] = vResult[2] = vResult[3] = v[2];
    return vResult;
}

inline Float4 VectorSplatW(const Float4& v) noexcept
{
    Float4 vResult;
    vResult[0] = vResult[1] = vResult[2] = vResult[3] = v[3];
    return vResult;
}

inline Float4 Vector3TransformNormal(const Float4& vector, const Matrix4x4& mat)
{
    Float4 Z = VectorSplatZ(vector);
    Float4 Y = VectorSplatY(vector);
    Float4 X = VectorSplatX(vector);

    Float4 Result = Float4_Multiply(Z, mat.GetFloat4(2));
    Result = Float4_Multiply(Y, mat.GetFloat4(1)) + Result;
    Result = Float4_Multiply(X, mat.GetFloat4(0)) + Result;

    return Result;
}

inline Float4 Vector3TransformCoord(const Float4& V, const Matrix4x4& M)
{
    Float4 Z = VectorSplatZ(V);
    Float4 Y = VectorSplatY(V);
    Float4 X = VectorSplatX(V);

    Float4 Result = Float4_Multiply(Z, M.GetFloat4(2)) + M.GetFloat4(3);
    Result = Float4_Multiply(Y, M.GetFloat4(1)) + Result;
    Result = Float4_Multiply(X, M.GetFloat4(0)) + Result;

    Float4 W = VectorSplatW(Result);
    return Float4_Divide(Result, W);
}

inline Matrix4x4 Matrix4x4_LookToLH
(
    const Float3& EyePosition,
    const Float3& EyeDirection,
    const Float3& upDirection
) noexcept
{
  /*  assert(!XMVector3Equal(EyeDirection, XMVectorZero()));
    assert(!XMVector3IsInfinite(EyeDirection));
    assert(!XMVector3Equal(UpDirection, XMVectorZero()));
    assert(!XMVector3IsInfinite(UpDirection));*/

    auto R2 = VectorNormalize(GetFloat3(EyeDirection));

    auto R0 = VectorCross(GetFloat3(upDirection), R2);
    R0 = VectorNormalize(R0);

    auto R1 = VectorCross(R2, R0);

    auto NegEyePosition = VectorNegate(GetFloat3(EyePosition));

    auto D0 = VectorDot(R0, NegEyePosition);
    auto D1 = VectorDot(R1, NegEyePosition);
    auto D2 = VectorDot(R2, NegEyePosition);

    Matrix4x4 M;
    M.GetFloat4(0) = VectorSelect(D0, R0, g_Select1110);
    M.GetFloat4(1) = VectorSelect(D1, R1, g_Select1110);
    M.GetFloat4(2) = VectorSelect(D2, R2, g_Select1110);
    M.GetFloat4(3) = Float4(0.f, 0, 0, 1);

    M = Matrix4x4_Transpose(M);

    return M;
}

inline Matrix4x4 Matrix4x4_LookAtLH(Float3 eyePos, Float3 focusPos, Float3 upDir)
{
    auto negEyeDir = focusPos - eyePos;
    return Matrix4x4_LookToLH(eyePos, negEyeDir, upDir);
}

inline Matrix4x4 Matrix4x4_OrthographicOffCenterLH
(
    float ViewLeft,
    float ViewRight,
    float ViewBottom,
    float ViewTop,
    float NearZ,
    float FarZ
) noexcept
{
    /* assert(!XMScalarNearEqual(ViewRight, ViewLeft, 0.00001f));
     assert(!XMScalarNearEqual(ViewTop, ViewBottom, 0.00001f));
     assert(!XMScalarNearEqual(FarZ, NearZ, 0.00001f));*/


    float ReciprocalWidth = 1.0f / (ViewRight - ViewLeft);
    float ReciprocalHeight = 1.0f / (ViewTop - ViewBottom);
    float fRange = 1.0f / (FarZ - NearZ);

    Matrix4x4 M;
    M.m[0][0] = ReciprocalWidth + ReciprocalWidth;
    M.m[0][1] = 0.0f;
    M.m[0][2] = 0.0f;
    M.m[0][3] = 0.0f;

    M.m[1][0] = 0.0f;
    M.m[1][1] = ReciprocalHeight + ReciprocalHeight;
    M.m[1][2] = 0.0f;
    M.m[1][3] = 0.0f;

    M.m[2][0] = 0.0f;
    M.m[2][1] = 0.0f;
    M.m[2][2] = fRange;
    M.m[2][3] = 0.0f;

    M.m[3][0] = -(ViewLeft + ViewRight) * ReciprocalWidth;
    M.m[3][1] = -(ViewTop + ViewBottom) * ReciprocalHeight;
    M.m[3][2] = -fRange * NearZ;
    M.m[3][3] = 1.0f;
    return M;
}

inline Float4 VectorSwizzle
(
    Float4 V,
    uint32_t E0,
    uint32_t E1,
    uint32_t E2,
    uint32_t E3
) noexcept
{
    assert((E0 < 4) && (E1 < 4) && (E2 < 4) && (E3 < 4));
    _Analysis_assume_((E0 < 4) && (E1 < 4) && (E2 < 4) && (E3 < 4));

    Float4 Result = {
            V[E0],
            V[E1],
            V[E2],
            V[E3]
    };
    return Result;
}

inline Float4 VectorNegativeMultiplySubtract
(
    Float4 V1,
    Float4 V2,
    Float4 V3
) noexcept
{
    Float4 Result = {
            V3[0] - (V1[0] * V2[0]),
            V3[1] - (V1[1] * V2[1]),
            V3[2] - (V1[2] * V2[2]),
            V3[3] - (V1[3] * V2[3])
    };
    return Result;
}

inline Float4 VectorMultiplyAdd
(
    Float4 V1,
    Float4 V2,
    Float4 V3
) noexcept
{
    Float4 Result =  {
            V1[0] * V2[0] + V3[0],
            V1[1] * V2[1] + V3[1],
            V1[2] * V2[2] + V3[2],
            V1[3] * V2[3] + V3[3]
        };
    return Result;
}

inline Float4 VectorPermute
(
    Float4 V1,
    Float4 V2,
    uint32_t PermuteX,
    uint32_t PermuteY,
    uint32_t PermuteZ,
    uint32_t PermuteW
) noexcept
{
    const uint32_t* aPtr[2];
    aPtr[0] = reinterpret_cast<const uint32_t*>(&V1);
    aPtr[1] = reinterpret_cast<const uint32_t*>(&V2);

    Float4 Result;
    auto pWork = reinterpret_cast<uint32_t*>(&Result);

    const uint32_t i0 = PermuteX & 3;
    const uint32_t vi0 = PermuteX >> 2;
    pWork[0] = aPtr[vi0][i0];

    const uint32_t i1 = PermuteY & 3;
    const uint32_t vi1 = PermuteY >> 2;
    pWork[1] = aPtr[vi1][i1];

    const uint32_t i2 = PermuteZ & 3;
    const uint32_t vi2 = PermuteZ >> 2;
    pWork[2] = aPtr[vi2][i2];

    const uint32_t i3 = PermuteW & 3;
    const uint32_t vi3 = PermuteW >> 2;
    pWork[3] = aPtr[vi3][i3];

    return Result;
}

inline Float4 VectorReciprocal(Float4 V) noexcept
{
    Float4 Result = {
            1.f / V[0],
            1.f / V[1],
            1.f / V[2],
            1.f / V[3]
    };
    return Result;
}

inline Float4 Matrix4x4_Determinant(const Matrix4x4& M) noexcept
{
    constexpr uint32_t XM_SWIZZLE_X = 0;
    constexpr uint32_t XM_SWIZZLE_Y = 1;
    constexpr uint32_t XM_SWIZZLE_Z = 2;
    constexpr uint32_t XM_SWIZZLE_W = 3;
    static const Float4 Sign = { 1.0f, -1.0f, 1.0f, -1.0f };

    auto V0 = VectorSwizzle(M.GetFloat4(2), XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_X);
    auto V1 = VectorSwizzle(M.GetFloat4(3), XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_Y);
    auto V2 = VectorSwizzle(M.GetFloat4(2), XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_X);
    auto V3 = VectorSwizzle(M.GetFloat4(3), XM_SWIZZLE_W, XM_SWIZZLE_W, XM_SWIZZLE_W, XM_SWIZZLE_Z);
    auto V4 = VectorSwizzle(M.GetFloat4(2), XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_Y);
    auto V5 = VectorSwizzle(M.GetFloat4(3), XM_SWIZZLE_W, XM_SWIZZLE_W, XM_SWIZZLE_W, XM_SWIZZLE_Z);

    auto P0 = Float4_Multiply(V0, V1);
    auto P1 = Float4_Multiply(V2, V3);
    auto P2 = Float4_Multiply(V4, V5);

    V0 = VectorSwizzle(M.GetFloat4(2), XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_Y);
    V1 = VectorSwizzle(M.GetFloat4(3), XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_X);
    V2 = VectorSwizzle(M.GetFloat4(2), XM_SWIZZLE_W, XM_SWIZZLE_W, XM_SWIZZLE_W, XM_SWIZZLE_Z);
    V3 = VectorSwizzle(M.GetFloat4(3), XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_X);
    V4 = VectorSwizzle(M.GetFloat4(2), XM_SWIZZLE_W, XM_SWIZZLE_W, XM_SWIZZLE_W, XM_SWIZZLE_Z);
    V5 = VectorSwizzle(M.GetFloat4(3), XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_Y);

    P0 = VectorNegativeMultiplySubtract(V0, V1, P0);
    P1 = VectorNegativeMultiplySubtract(V2, V3, P1);
    P2 = VectorNegativeMultiplySubtract(V4, V5, P2);

    V0 = VectorSwizzle(M.GetFloat4(1), XM_SWIZZLE_W, XM_SWIZZLE_W, XM_SWIZZLE_W, XM_SWIZZLE_Z);
    V1 = VectorSwizzle(M.GetFloat4(1), XM_SWIZZLE_Z, XM_SWIZZLE_Z, XM_SWIZZLE_Y, XM_SWIZZLE_Y);
    V2 = VectorSwizzle(M.GetFloat4(1), XM_SWIZZLE_Y, XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_X);

    Float4 S = Float4_Multiply(M.GetFloat4(0), Sign);
    Float4 R = Float4_Multiply(V0, P0);
    R = VectorNegativeMultiplySubtract(V1, P1, R);
    R = VectorMultiplyAdd(V2, P2, R);

    return VectorDot(S, R);
}

inline Matrix4x4 Matrix4x4_Inverse(Float4* pDeterminant, const Matrix4x4&  M) noexcept
{
    constexpr uint32_t XM_SWIZZLE_X = 0;
    constexpr uint32_t XM_SWIZZLE_Y = 1;
    constexpr uint32_t XM_SWIZZLE_Z = 2;
    constexpr uint32_t XM_SWIZZLE_W = 3;

    constexpr uint32_t XM_PERMUTE_0X = 0;
    constexpr uint32_t XM_PERMUTE_0Y = 1;
    constexpr uint32_t XM_PERMUTE_0Z = 2;
    constexpr uint32_t XM_PERMUTE_0W = 3;
    constexpr uint32_t XM_PERMUTE_1X = 4;
    constexpr uint32_t XM_PERMUTE_1Y = 5;
    constexpr uint32_t XM_PERMUTE_1Z = 6;
    constexpr uint32_t XM_PERMUTE_1W = 7;

    auto MT = Matrix4x4_Transpose(M);

    Float4 V0[4], V1[4];
    V0[0] = VectorSwizzle(MT.GetFloat4(2), XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_Y, XM_SWIZZLE_Y);
    V1[0] = VectorSwizzle(MT.GetFloat4(3), XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_W);
    V0[1] = VectorSwizzle(MT.GetFloat4(0), XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_Y, XM_SWIZZLE_Y);
    V1[1] = VectorSwizzle(MT.GetFloat4(1), XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_W);
    V0[2] = VectorPermute(MT.GetFloat4(2), MT.GetFloat4(0), XM_PERMUTE_0X, XM_PERMUTE_0Z, XM_PERMUTE_1X, XM_PERMUTE_1Z);
    V1[2] = VectorPermute(MT.GetFloat4(3), MT.GetFloat4(1), XM_PERMUTE_0Y, XM_PERMUTE_0W, XM_PERMUTE_1Y, XM_PERMUTE_1W);

    auto D0 = Float4_Multiply(V0[0], V1[0]);
    auto D1 = Float4_Multiply(V0[1], V1[1]);
    auto D2 = Float4_Multiply(V0[2], V1[2]);

    V0[0] = VectorSwizzle(MT.GetFloat4(2), XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_W);
    V1[0] = VectorSwizzle(MT.GetFloat4(3), XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_Y, XM_SWIZZLE_Y);
    V0[1] = VectorSwizzle(MT.GetFloat4(0), XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_W);
    V1[1] = VectorSwizzle(MT.GetFloat4(1), XM_SWIZZLE_X, XM_SWIZZLE_X, XM_SWIZZLE_Y, XM_SWIZZLE_Y);
    V0[2] = VectorPermute(MT.GetFloat4(2), MT.GetFloat4(0), XM_PERMUTE_0Y, XM_PERMUTE_0W, XM_PERMUTE_1Y, XM_PERMUTE_1W);
    V1[2] = VectorPermute(MT.GetFloat4(3), MT.GetFloat4(1), XM_PERMUTE_0X, XM_PERMUTE_0Z, XM_PERMUTE_1X, XM_PERMUTE_1Z);

    D0 = VectorNegativeMultiplySubtract(V0[0], V1[0], D0);
    D1 = VectorNegativeMultiplySubtract(V0[1], V1[1], D1);
    D2 = VectorNegativeMultiplySubtract(V0[2], V1[2], D2);

    V0[0] = VectorSwizzle(MT.GetFloat4(1), XM_SWIZZLE_Y, XM_SWIZZLE_Z, XM_SWIZZLE_X, XM_SWIZZLE_Y);
    V1[0] = VectorPermute(D0, D2, XM_PERMUTE_1Y, XM_PERMUTE_0Y, XM_PERMUTE_0W, XM_PERMUTE_0X);
    V0[1] = VectorSwizzle(MT.GetFloat4(0), XM_SWIZZLE_Z, XM_SWIZZLE_X, XM_SWIZZLE_Y, XM_SWIZZLE_X);
    V1[1] = VectorPermute(D0, D2, XM_PERMUTE_0W, XM_PERMUTE_1Y, XM_PERMUTE_0Y, XM_PERMUTE_0Z);
    V0[2] = VectorSwizzle(MT.GetFloat4(3), XM_SWIZZLE_Y, XM_SWIZZLE_Z, XM_SWIZZLE_X, XM_SWIZZLE_Y);
    V1[2] = VectorPermute(D1, D2, XM_PERMUTE_1W, XM_PERMUTE_0Y, XM_PERMUTE_0W, XM_PERMUTE_0X);
    V0[3] = VectorSwizzle(MT.GetFloat4(2), XM_SWIZZLE_Z, XM_SWIZZLE_X, XM_SWIZZLE_Y, XM_SWIZZLE_X);
    V1[3] = VectorPermute(D1, D2, XM_PERMUTE_0W, XM_PERMUTE_1W, XM_PERMUTE_0Y, XM_PERMUTE_0Z);

    auto C0 = Float4_Multiply(V0[0], V1[0]);
    auto C2 = Float4_Multiply(V0[1], V1[1]);
    auto C4 = Float4_Multiply(V0[2], V1[2]);
    auto C6 = Float4_Multiply(V0[3], V1[3]);

    V0[0] = VectorSwizzle(MT.GetFloat4(1), XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_Y, XM_SWIZZLE_Z);
    V1[0] = VectorPermute(D0, D2, XM_PERMUTE_0W, XM_PERMUTE_0X, XM_PERMUTE_0Y, XM_PERMUTE_1X);
    V0[1] = VectorSwizzle(MT.GetFloat4(0), XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_Y);
    V1[1] = VectorPermute(D0, D2, XM_PERMUTE_0Z, XM_PERMUTE_0Y, XM_PERMUTE_1X, XM_PERMUTE_0X);
    V0[2] = VectorSwizzle(MT.GetFloat4(3), XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_Y, XM_SWIZZLE_Z);
    V1[2] = VectorPermute(D1, D2, XM_PERMUTE_0W, XM_PERMUTE_0X, XM_PERMUTE_0Y, XM_PERMUTE_1Z);
    V0[3] = VectorSwizzle(MT.GetFloat4(2), XM_SWIZZLE_W, XM_SWIZZLE_Z, XM_SWIZZLE_W, XM_SWIZZLE_Y);
    V1[3] = VectorPermute(D1, D2, XM_PERMUTE_0Z, XM_PERMUTE_0Y, XM_PERMUTE_1Z, XM_PERMUTE_0X);

    C0 = VectorNegativeMultiplySubtract(V0[0], V1[0], C0);
    C2 = VectorNegativeMultiplySubtract(V0[1], V1[1], C2);
    C4 = VectorNegativeMultiplySubtract(V0[2], V1[2], C4);
    C6 = VectorNegativeMultiplySubtract(V0[3], V1[3], C6);

    V0[0] = VectorSwizzle(MT.GetFloat4(1), XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_X);
    V1[0] = VectorPermute(D0, D2, XM_PERMUTE_0Z, XM_PERMUTE_1Y, XM_PERMUTE_1X, XM_PERMUTE_0Z);
    V0[1] = VectorSwizzle(MT.GetFloat4(0), XM_SWIZZLE_Y, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Z);
    V1[1] = VectorPermute(D0, D2, XM_PERMUTE_1Y, XM_PERMUTE_0X, XM_PERMUTE_0W, XM_PERMUTE_1X);
    V0[2] = VectorSwizzle(MT.GetFloat4(3), XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_W, XM_SWIZZLE_X);
    V1[2] = VectorPermute(D1, D2, XM_PERMUTE_0Z, XM_PERMUTE_1W, XM_PERMUTE_1Z, XM_PERMUTE_0Z);
    V0[3] = VectorSwizzle(MT.GetFloat4(2), XM_SWIZZLE_Y, XM_SWIZZLE_W, XM_SWIZZLE_X, XM_SWIZZLE_Z);
    V1[3] = VectorPermute(D1, D2, XM_PERMUTE_1W, XM_PERMUTE_0X, XM_PERMUTE_0W, XM_PERMUTE_1Z);

    auto C1 = VectorNegativeMultiplySubtract(V0[0], V1[0], C0);
    C0 = VectorMultiplyAdd(V0[0], V1[0], C0);
    auto C3 = VectorMultiplyAdd(V0[1], V1[1], C2);
    C2 = VectorNegativeMultiplySubtract(V0[1], V1[1], C2);
    auto C5 = VectorNegativeMultiplySubtract(V0[2], V1[2], C4);
    C4 = VectorMultiplyAdd(V0[2], V1[2], C4);
    auto C7 = VectorMultiplyAdd(V0[3], V1[3], C6);
    C6 = VectorNegativeMultiplySubtract(V0[3], V1[3], C6);

    Matrix4x4 R;
    constexpr uint32_t XM_SELECT_0 = 0x00000000;
    constexpr uint32_t XM_SELECT_1 = 0xFFFFFFFF;
    Float4 select0101 = { XM_SELECT_0, XM_SELECT_1, XM_SELECT_0, XM_SELECT_1 };
    R.GetFloat4(0) = VectorSelect(C0, C1, select0101);
    R.GetFloat4(1) = VectorSelect(C2, C3, select0101);
    R.GetFloat4(2) = VectorSelect(C4, C5, select0101);
    R.GetFloat4(3) = VectorSelect(C6, C7, select0101);

    auto Determinant = VectorDot(R.GetFloat4(0), MT.GetFloat4(0));

    if (pDeterminant != nullptr)
        *pDeterminant = Determinant;

    Float4 Reciprocal = VectorReciprocal(Determinant);

    Matrix4x4 Result;
    Result.GetFloat4(0) = Float4_Multiply(R.GetFloat4(0), Reciprocal);
    Result.GetFloat4(1) = Float4_Multiply(R.GetFloat4(1), Reciprocal);
    Result.GetFloat4(2) = Float4_Multiply(R.GetFloat4(2), Reciprocal);
    Result.GetFloat4(3) = Float4_Multiply(R.GetFloat4(3), Reciprocal);
    return Result;
}