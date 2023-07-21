#pragma once

struct Vector2
{
	float x = 0.f, y = 0.f;

	Vector2() {}

	Vector2(float x, float y) : x(x), y(y) {}

	Vector2 operator+(const Vector2& other)
	{
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 operator*(float other)
	{
		return Vector2(x * other, y * other);
	}
};

inline Vector2 operator*(float other, Vector2 v) noexcept
{
	return Vector2(v.x * other, v.y * other);
}

struct Vector3
{
	float x = 0.f, y = 0.f, z = 0.f;

	Vector3() {};

	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vector3 operator+(const Vector3& other)
	{
		return Vector3(x + other.x, y + other.y, z + other.z);
	}

	Vector3 Dot(const Vector3& other)
	{
		float fValue = x * other.x + y * other.y + z * other.z;
		return { fValue, fValue, fValue };
	}

	Vector3 LengthSq()
	{
		return Dot(*this);
	}

	Vector3 Sqrt()
	{
		return { x * x, y * y, z * z };
	}

	Vector3 Length()
	{
		Vector3 Result;

		Result = LengthSq();
		Result = Result.Sqrt();

		return Result;
	}

	Vector3 Normalize()
	{
		float fLength;
		Vector3 vResult;

		vResult = Length();
		fLength = vResult.x;

		// Prevent divide by zero
		if (fLength > 0)
		{
			fLength = 1.0f / fLength;
		}

		vResult.x = x * fLength;
		vResult.y = y * fLength;
		vResult.z = z * fLength;
		return vResult;
	}
};

inline Vector3 operator*(float other, Vector3 v) noexcept
{
	return Vector3(v.x * other, v.y * other, v.z * other);
}
