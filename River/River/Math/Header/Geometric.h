#pragma once
#include "BaseDefine.h"
#include "BaseStruct.h"

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

inline Float4 VectorMergeXY(Float4& v1, Float4& v2)
{
    Float4 Result = {
        v1[0],
        v2[0],
        v1[1],
        v2[1],
    };
return Result;

}

inline Float4 VectorMergeZW(Float4& v1, Float4& v2)
{

    Float4 Result = {
            v1[2],
            v2[2],
            v1[3],
            v2[3]
        };
    return Result;
}

inline Matrix4x4 Matrix4x4_Transpose(Matrix4x4& mat)
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

inline Float4 VectorSplatX(Float4& V) noexcept
{
    Float4 vResult;
    vResult[0] = vResult[1] = vResult[2] = vResult[3] = V[0];
    return vResult;
}

inline Float4 VectorSplatY(Float4& v) noexcept
{
    Float4 vResult;
    vResult[0] = vResult[1] = vResult[2] = vResult[3] = v[1];
    return vResult;
}

inline Float4 VectorSplatZ(Float4& v) noexcept
{
    Float4 vResult;
    vResult[0] = vResult[1] = vResult[2] = vResult[3] = v[2];
    return vResult;
}

inline Float4 Vector3TransformNormal(Float4& vector, Matrix4x4& mat)
{
    Float4 Z = VectorSplatZ(vector);
    Float4 Y = VectorSplatY(vector);
    Float4 X = VectorSplatX(vector);

    Float4 Result = Float4_Multiply(Z, mat.GetFloat4(2));
    Result = Float4_Multiply(Y, mat.GetFloat4(1)) + Result;
    Result = Float4_Multiply(X, mat.GetFloat4(0)) + Result;

    return Result;
}