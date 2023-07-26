#pragma once

#define PI 3.141592654f
#define _2_PI 6.283185307f
#define PI_DIV_2 1.570796327f

inline float DegreeToRadians(float degree) noexcept
{
	return degree * (PI / 180.0f);
}

inline float RadiansToDegree(float radians) noexcept
{
	return radians * (180.0f / PI);
}

inline int Rand(int a, int b)
{
	return a + rand() % ((b - a) + 1);
}

inline float RandF()
{
	return (float)(rand()) / (float)RAND_MAX;
}

inline float RandF(float a, float b)
{
	return a + RandF() * (b - a);
}

template<typename T>
static T Clamp(const T& x, const T& low, const T& high)
{
	return x < low ? low : (x > high ? high : x);
}