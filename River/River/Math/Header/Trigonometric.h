#pragma once
#include "BaseDefine.h"

inline float DegreeToRadians(float degree) noexcept
{
	return degree * (MATH_PI / 180.0f);
}

inline float RadiansToDegree(float radians) noexcept
{
	return radians * (180.0f / MATH_PI);
}
