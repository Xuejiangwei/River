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
    float fValue = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
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

inline Float4 VectorLess(Float4 v1,Float4 v2) noexcept
{
    return Float4(
        (v1[0] < v2[0]) ? 0xFFFFFFFF : 0,
        (v1[1] < v2[1]) ? 0xFFFFFFFF : 0,
        (v1[2] < v2[2]) ? 0xFFFFFFFF : 0,
        (v1[3] < v2[3]) ? 0xFFFFFFFF : 0);
}

inline Float4 VectorATan2(Float4 y, Float4 x) noexcept
{
    return Float4(
        atan2f(y[0], x[0]),
        atan2f(y[1], x[1]),
        atan2f(y[2], x[2]),
        atan2f(y[3], x[3]));
}

inline Float4 VectorXorInt(Float4 v1, Float4 v2) noexcept
{
    return Float4(v1.ux ^ v2.ux, v1.uy ^ v2.uy, v1.uz ^ v2.uz, v1.uw ^ v2.uw);
}
inline Float4 VectorReciprocal(Float4 v) noexcept
{
    return Float4(1.f / v[0], 1.f / v[1], 1.f / v[2], 1.f / v[3]);
}

inline Float4 VectorSin(Float4 v) noexcept
{
    return Float4(sinf(v[0]), sinf(v[1]), sinf(v[2]), sinf(v[3]));
}

inline Float4 VectorToQuaternion(const Float4& rot) noexcept
{
    float cy = cos(rot.y * 0.5);
    float sy = sin(rot.y * 0.5);
    float cp = cos(rot.z * 0.5);
    float sp = sin(rot.z * 0.5);
    float cr = cos(rot.x * 0.5);
    float sr = sin(rot.x * 0.5);

    return { cy * cp * cr + sy * sp * sr, cy * cp * sr - sy * sp * cr,
             sy * cp * sr + cy * sp * cr, sy * cp * cr - cy * sp * sr };
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

inline Float4  VectorSplatSignMask() noexcept
{
    return Float4(0x80000000U, 0x80000000U, 0x80000000U, 0x80000000U);
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

inline Float4 Vector3TransformCoord(const Float4& v, const Matrix4x4& m)
{
    Float4 Z = VectorSplatZ(v);
    Float4 Y = VectorSplatY(v);
    Float4 X = VectorSplatX(v);

    Float4 Result = Float4_Multiply(Z, m.GetFloat4(2)) + m.GetFloat4(3);
    Result = Float4_Multiply(Y, m.GetFloat4(1)) + Result;
    Result = Float4_Multiply(X, m.GetFloat4(0)) + Result;

    Float4 W = VectorSplatW(Result);
    return Float4_Divide(Result, W);
}

inline Matrix4x4 Matrix4x4_LookToLH(const Float3& eyePosition, const Float3& eyeDirection, const Float3& upDirection) noexcept
{
    auto R2 = VectorNormalize(GetFloat3(eyeDirection));

    auto R0 = VectorCross(GetFloat3(upDirection), R2);
    R0 = VectorNormalize(R0);

    auto R1 = VectorCross(R2, R0);

    auto NegEyePosition = VectorNegate(GetFloat3(eyePosition));

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

inline Matrix4x4 Matrix4x4_OrthographicOffCenterLH(float viewLeft, float viewRight, float viewBottom, float viewTop,
    float nearZ, float farZ) noexcept
{
    float ReciprocalWidth = 1.0f / (viewRight - viewLeft);
    float ReciprocalHeight = 1.0f / (viewTop - viewBottom);
    float fRange = 1.0f / (farZ - nearZ);

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

    M.m[3][0] = -(viewLeft + viewRight) * ReciprocalWidth;
    M.m[3][1] = -(viewTop + viewBottom) * ReciprocalHeight;
    M.m[3][2] = -fRange * nearZ;
    M.m[3][3] = 1.0f;
    return M;
}

inline Float4 VectorSwizzle(Float4 v, uint32 e0, uint32 e1, uint32 e2, uint32 e3) noexcept
{
    return Float4(v[e0], v[e1], v[e2], v[e3]);
}

inline Float4 VectorNegativeMultiplySubtract(Float4 v1, Float4 v2, Float4 v3) noexcept
{
    return Float4(v3[0] - (v1[0] * v2[0]), v3[1] - (v1[1] * v2[1]), v3[2] - (v1[2] * v2[2]), v3[3] - (v1[3] * v2[3]));
}

inline Float4 VectorMultiplyAdd(const Float4& v1, const Float4& v2, const Float4& v3) noexcept
{
    return Float4(v1[0] * v2[0] + v3[0], v1[1] * v2[1] + v3[1], v1[2] * v2[2] + v3[2], v1[3] * v2[3] + v3[3]);
}

inline Float4 VectorLerp(Float4 v0, Float4 v1, float t)
{
    auto scale = Float4(t, t, t, t);
    auto length = Float4_Sub(v1, v0);
    return VectorMultiplyAdd(length, scale, v0);
}

inline Float4 VectorPermute(Float4 v1, Float4 v2, uint32 permuteX, uint32 permuteY, uint32 permuteZ, uint32_t permuteW) noexcept
{
    const uint32_t* aPtr[2];
    aPtr[0] = reinterpret_cast<const uint32_t*>(&v1);
    aPtr[1] = reinterpret_cast<const uint32_t*>(&v2);

    Float4 Result;
    auto pWork = reinterpret_cast<uint32_t*>(&Result);

    const uint32_t i0 = permuteX & 3;
    const uint32_t vi0 = permuteX >> 2;
    pWork[0] = aPtr[vi0][i0];

    const uint32_t i1 = permuteY & 3;
    const uint32_t vi1 = permuteY >> 2;
    pWork[1] = aPtr[vi1][i1];

    const uint32_t i2 = permuteZ & 3;
    const uint32_t vi2 = permuteZ >> 2;
    pWork[2] = aPtr[vi2][i2];

    const uint32_t i3 = permuteW & 3;
    const uint32_t vi3 = permuteW >> 2;
    pWork[3] = aPtr[vi3][i3];

    return Result;
}

inline Float4 VectorShiftLeft(Float4 v1, Float4 v2, uint32 elements) noexcept
{
    return VectorPermute(v1, v2, elements, ((elements)+1), ((elements)+2), ((elements)+3));
}

inline Float4  QuaternionSlerpV(Float4 q0, Float4 q1, Float4 t) noexcept
{
    const Float4 OneMinusEpsilon(1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f, 1.0f - 0.00001f);

    Float4 CosOmega = VectorDot(q0, q1);

    Float4 Zero;
    Float4 Control = VectorLess(CosOmega, Zero);
    Float4 Sign = VectorSelect(Float4(1.0f, 1.0f, 1.0f, 1.0f), Float4(-1.0f, -1.0f, -1.0f, -1.0f), Control);

    CosOmega = Float4_Multiply(CosOmega, Sign);

    Control = VectorLess(CosOmega, OneMinusEpsilon);

    Float4 SinOmega = VectorNegativeMultiplySubtract(CosOmega, CosOmega, Float4(1.0f, 1.0f, 1.0f, 1.0f));
    SinOmega = VectorSqrt(SinOmega);

    Float4 Omega = VectorATan2(SinOmega, CosOmega);

    Float4 SignMask = VectorSplatSignMask();
    Float4 V01 = VectorShiftLeft(t, Zero, 2);
    SignMask = VectorShiftLeft(SignMask, Zero, 3);
    V01 = VectorXorInt(V01, SignMask);
    V01 = Float4_Add(Float4(1.0f, 0.0f, 0.0f, 0.0f), V01);

    Float4 InvSinOmega = VectorReciprocal(SinOmega);

    Float4 S0 = Float4_Multiply(V01, Omega);
    S0 = VectorSin(S0);
    S0 = Float4_Multiply(S0, InvSinOmega);

    S0 = VectorSelect(V01, S0, Control);

    Float4 S1 = VectorSplatY(S0);
    S0 = VectorSplatX(S0);

    S1 = Float4_Multiply(S1, Sign);

    Float4 Result = Float4_Multiply(q0, S0);
    Result = VectorMultiplyAdd(q1, S1, Result);

    return Result;
}

inline Float4 QuaternionSlerp(Float4 q0, Float4 q1, float t) noexcept
{
    return QuaternionSlerpV(q0, q1, Float4(t, t, t, t));
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

inline Matrix4x4 Matrix4x4_RotationNormal
(
    Float3 NormalAxis,
    float     Angle
) noexcept
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

    float    fSinAngle;
    float    fCosAngle;
    Matrix4x4_ScalarSinCos(&fSinAngle, &fCosAngle, Angle);

    auto A = Float4(fSinAngle, fCosAngle, 1.0f - fCosAngle, 0.0f);

    auto C2 = VectorSplatZ(A);
    auto C1 = VectorSplatY(A);
    auto C0 = VectorSplatX(A);

    auto N0 = VectorSwizzle(NormalAxis, XM_SWIZZLE_Y, XM_SWIZZLE_Z, XM_SWIZZLE_X, XM_SWIZZLE_W);
    auto N1 = VectorSwizzle(NormalAxis, XM_SWIZZLE_Z, XM_SWIZZLE_X, XM_SWIZZLE_Y, XM_SWIZZLE_W);

    auto V0 = Float4_Multiply(C2, N0);
    V0 = Float4_Multiply(V0, N1);

    auto R0 = Float4_Multiply(C2, NormalAxis);
    R0 = VectorMultiplyAdd(R0, NormalAxis, C1);

    auto R1 = VectorMultiplyAdd(C0, NormalAxis, V0);
    auto R2 = VectorNegativeMultiplySubtract(C0, NormalAxis, V0);

    V0 = VectorSelect(A, R0, g_Select1110);
    auto V1 = VectorPermute(R1, R2, XM_PERMUTE_0Z, XM_PERMUTE_1Y, XM_PERMUTE_1Z, XM_PERMUTE_0X);
    auto V2 = VectorPermute(R1, R2, XM_PERMUTE_0Y, XM_PERMUTE_1X, XM_PERMUTE_0Y, XM_PERMUTE_1X);

    Matrix4x4 M;
    M.GetFloat4(0) = VectorPermute(V0, V1, XM_PERMUTE_0X, XM_PERMUTE_1X, XM_PERMUTE_1Y, XM_PERMUTE_0W);
    M.GetFloat4(1) = VectorPermute(V0, V1, XM_PERMUTE_1Z, XM_PERMUTE_0Y, XM_PERMUTE_1W, XM_PERMUTE_0W);
    M.GetFloat4(2) = VectorPermute(V0, V2, XM_PERMUTE_1X, XM_PERMUTE_1Y, XM_PERMUTE_0Z, XM_PERMUTE_0W);
    M.GetFloat4(3) = { 0.0f, 0.0f, 0.0f, 1.0f };
    return M;
}

inline Matrix4x4 Matrix4x4_RotationAxis(const Float3& axis, float angle)
{
    assert(!axis.IsZero());
    assert(!axis.IsInfinit());

    return Matrix4x4_RotationNormal(axis.Normalize(), angle);
}

inline Matrix4x4 Matrix4x4_RotationQuaternion(Float4 Quaternion) noexcept
{
    float qx = Quaternion[0];
    float qxx = qx * qx;

    float qy = Quaternion[1];
    float qyy = qy * qy;

    float qz = Quaternion[2];
    float qzz = qz * qz;

    float qw = Quaternion[3];

    Matrix4x4 M;
    M.m[0][0] = 1.f - 2.f * qyy - 2.f * qzz;
    M.m[0][1] = 2.f * qx * qy + 2.f * qz * qw;
    M.m[0][2] = 2.f * qx * qz - 2.f * qy * qw;
    M.m[0][3] = 0.f;

    M.m[1][0] = 2.f * qx * qy - 2.f * qz * qw;
    M.m[1][1] = 1.f - 2.f * qxx - 2.f * qzz;
    M.m[1][2] = 2.f * qy * qz + 2.f * qx * qw;
    M.m[1][3] = 0.f;

    M.m[2][0] = 2.f * qx * qz + 2.f * qy * qw;
    M.m[2][1] = 2.f * qy * qz - 2.f * qx * qw;
    M.m[2][2] = 1.f - 2.f * qxx - 2.f * qyy;
    M.m[2][3] = 0.f;

    M.m[3][0] = 0.f;
    M.m[3][1] = 0.f;
    M.m[3][2] = 0.f;
    M.m[3][3] = 1.0f;
    return M;
}

inline Matrix4x4 Matrix4x4_PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
{
    float    SinFov;
    float    CosFov;
    Matrix4x4_ScalarSinCos(&SinFov, &CosFov, 0.5f * FovAngleY);

    float Height = CosFov / SinFov;
    float Width = Height / AspectRatio;
    float fRange = FarZ / (FarZ - NearZ);

    Matrix4x4 M;
    M.m[0][0] = Width;
    M.m[0][1] = 0.0f;
    M.m[0][2] = 0.0f;
    M.m[0][3] = 0.0f;

    M.m[1][0] = 0.0f;
    M.m[1][1] = Height;
    M.m[1][2] = 0.0f;
    M.m[1][3] = 0.0f;

    M.m[2][0] = 0.0f;
    M.m[2][1] = 0.0f;
    M.m[2][2] = fRange;
    M.m[2][3] = 1.0f;

    M.m[3][0] = 0.0f;
    M.m[3][1] = 0.0f;
    M.m[3][2] = -fRange * NearZ;
    M.m[3][3] = 0.0f;
    return M;
}

inline Matrix4x4 Matrix4x4_OrthographicLH
(
    float ViewWidth,
    float ViewHeight,
    float NearZ,
    float FarZ
) noexcept
{
    float fRange = 1.0f / (FarZ - NearZ);

    Matrix4x4 M;
    M.m[0][0] = 2.0f / ViewWidth;
    M.m[0][1] = 0.0f;
    M.m[0][2] = 0.0f;
    M.m[0][3] = 0.0f;

    M.m[1][0] = 0.0f;
    M.m[1][1] = 2.0f / ViewHeight;
    M.m[1][2] = 0.0f;
    M.m[1][3] = 0.0f;

    M.m[2][0] = 0.0f;
    M.m[2][1] = 0.0f;
    M.m[2][2] = fRange;
    M.m[2][3] = 0.0f;

    M.m[3][0] = 0.0f;
    M.m[3][1] = 0.0f;
    M.m[3][2] = -fRange * NearZ;
    M.m[3][3] = 1.0f;
    return M;
}

inline Matrix4x4 Matrix4x4_Scaling
(
    float ScaleX,
    float ScaleY,
    float ScaleZ
) noexcept
{
    Matrix4x4 M;
    M.m[0][0] = ScaleX;
    M.m[0][1] = 0.0f;
    M.m[0][2] = 0.0f;
    M.m[0][3] = 0.0f;

    M.m[1][0] = 0.0f;
    M.m[1][1] = ScaleY;
    M.m[1][2] = 0.0f;
    M.m[1][3] = 0.0f;

    M.m[2][0] = 0.0f;
    M.m[2][1] = 0.0f;
    M.m[2][2] = ScaleZ;
    M.m[2][3] = 0.0f;

    M.m[3][0] = 0.0f;
    M.m[3][1] = 0.0f;
    M.m[3][2] = 0.0f;
    M.m[3][3] = 1.0f;
    return M;
}

inline Matrix4x4 Matrix4x4_ScalingFromVector(const Float3& Scale) noexcept
{
    return Matrix4x4_Scaling(Scale.x, Scale.y, Scale.z);
}

inline Matrix4x4 Matrix4x4_AffineTransformation
(
    Float4 Scaling,
    Float4 RotationOrigin,
    Float4 RotationQuaternion,
    Float4 Translation
    ) noexcept
{
    // M = MScaling * Inverse(MRotationOrigin) * MRotation * MRotationOrigin * MTranslation;

    auto MScaling = Matrix4x4_ScalingFromVector(Scaling);
    auto VRotationOrigin = VectorSelect(g_Select1110, RotationOrigin, g_Select1110);
    auto MRotation = Matrix4x4_RotationQuaternion(RotationQuaternion);
    auto VTranslation = VectorSelect(g_Select1110, Translation, g_Select1110);

    Matrix4x4 M;
    M = MScaling;
    M.GetFloat4(3) = Float4_Sub(M.GetFloat4(3), VRotationOrigin);
    M = Matrix4x4::Multiply(M, MRotation);
    M.GetFloat4(3) = Float4_Add(M.GetFloat4(3), VRotationOrigin);
    M.GetFloat4(3) = Float4_Add(M.GetFloat4(3), VTranslation);
    return M;
}