#pragma once

#define PI 3.141592654f
#define _2_PI 6.283185307f

inline float DegreeToRadians(float degree) noexcept
{
	return degree * (PI / 180.0f);
}

inline float RadiansToDegree(float radians) noexcept
{
	return radians * (180.0f / PI);
}