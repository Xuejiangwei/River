#pragma once

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