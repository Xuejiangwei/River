#pragma once
#include "Trigonometric.h"

struct Byte4
{
	unsigned char r, g, b, a;

	Byte4() : r(0), g(0), b(0), a(255) {}

	Byte4(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
		: r(r), g(g), b(b), a(a) {}

	operator unsigned int() const {
		return ((unsigned int)(r) << 0) | ((unsigned int)(g) << 8)
			| ((unsigned int)(g) << 16) | ((unsigned int)(a) << 24);
	}
};

struct UInt4
{
	union
	{
		struct
		{
			unsigned int x, y, z, w;
		};

		unsigned int v[4];
	};
};

template<typename T>
struct BaseStruct2
{
	T x;
	T y;

	BaseStruct2() : x(0), y(0) {}

	BaseStruct2(T x, T y) : x(x), y(y) {}

	BaseStruct2 operator+(const BaseStruct2& other)
	{
		return BaseStruct2(x + other.x, y + other.y);
	}

	BaseStruct2 operator*(T other)
	{
		return BaseStruct2(x * other, y * other);
	}

	BaseStruct2 operator*(BaseStruct2 other)
	{
		return BaseStruct2(x * other.x, y * other.y);
	}

	BaseStruct2& operator+=(const BaseStruct2& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}
};

template<typename T>
inline BaseStruct2<T> operator*(T other, BaseStruct2<T> v) noexcept
{
	return Float2(v.x * other, v.y * other);
}

using Float2 = BaseStruct2<float>;
using Int2 = BaseStruct2<int>;

struct Float3
{
	float x = 0.f, y = 0.f, z = 0.f;

	Float3() {};

	Float3(float x, float y, float z) : x(x), y(y), z(z) {}

	Float3(const struct Float4& other);

	Float3 operator+(const Float3& other) const
	{
		return Float3(x + other.x, y + other.y, z + other.z);
	}

	Float3 operator-(const Float3& other) const
	{
		return Float3(x - other.x, y - other.y, z - other.z);
	}

	float& operator[](int i)
	{
		return *((float*)this + i);
	}

	bool IsZero() const
	{
		return x == 0.0f && y == 0.0f && z == 0.0f;
	}

	bool IsInfinit() const
	{
		return ((*(const uint32_t*)&(x) & 0x7FFFFFFF) == 0x7F800000) || ((*(const uint32_t*)&(y) & 0x7FFFFFFF) == 0x7F800000)
			|| ((*(const uint32_t*)&(z) & 0x7FFFFFFF) == 0x7F800000);
	}

	Float3 Dot(const Float3& other) const
	{
		float fValue = x * other.x + y * other.y + z * other.z;
		return { fValue, fValue, fValue };
	}

	Float3 LengthSq() const
	{
		return Dot(*this);
	}

	Float3 Sqrt() const
	{
		return { x * x, y * y, z * z };
	}

	Float3 Length() const
	{
		Float3 Result;

		Result = LengthSq();
		Result = Result.Sqrt();

		return Result;
	}

	Float3 Normalize() const
	{
		float fLength;
		Float3 vResult;

		vResult = Length();
		fLength = vResult.x;

		if (fLength > 0)
		{
			fLength = 1.0f / fLength;
		}

		vResult.x = x * fLength;
		vResult.y = y * fLength;
		vResult.z = z * fLength;
		return vResult;
	}

	float Len() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	float LenSquared() const
	{
		return x * x + y * y + z * z;
	}

	Float3 operator+ (float n) const
	{
		return Float3(x + n, y + n, z + n);
	}

	Float3 operator- (float n) const
	{
		return Float3(x - n, y - n, z - n);
	}

	Float3 operator/ (float n) const
	{
		return Float3(x / n, y / n, z / n);
	}

	Float3 operator* (float n) const
	{
		return Float3(x * n, y * n, z * n);
	}

	inline Float3& operator=(const struct Float4& other) noexcept;
};

inline Float3 operator*(float other, Float3 v) noexcept
{
	return Float3(v.x * other, v.y * other, v.z * other);
}

struct Float4
{
	union
	{
		struct
		{
			float x, y, z, w;
		};

		struct
		{
			unsigned int ux, uy, uz, uw;
		};

		struct
		{
			float r, g, b, a;
		};
	};

	Float4()
	{
		x = y = z = w = 0;
	}

	Float4(float x, float y, float z, float w)
		: x(x), y(y), z(z), w(w)
	{}

	Float4(unsigned int x, unsigned int y, unsigned int z, unsigned int w)
		: ux(x), uy(y), uz(z), uw(w)
	{}

	Float4(const Float3& f3) : Float4(f3.x, f3.y, f3.z, 0.0f) {}

	Float4& operator+(const Float4& v) noexcept
	{
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		this->w += v.w;
		return *this;
	}

	Float4& operator-(const Float4& v) noexcept
	{
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.z;
		this->w -= v.w;
		return *this;
	}

	float& operator[](int i)
	{
		return *((float*)this + i);
	}

	float operator[](int i) const
	{
		return *((float*)this + i);
	}

	Float4 operator*(float v)
	{
		return { x * v, y * v, z * v, w * v };
	}

	Float4 operator/(float v)
	{
		return { x / v, y / v, z / v, w / v };
	}

	Float4 operator+=(const Float4& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	Float4 operator-=(const Float4& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	unsigned int& GetUintIndex(int i)
	{
		return *((unsigned int*)this + i);
	}
};

inline Float4 operator*(float other, Float4 v) noexcept
{
	return Float4(v.x * other, v.y * other, v.z * other, v.w * other);
}


inline Float4 Float4_Add(const Float4& v1, const Float4& v2) noexcept
{
	return Float4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

inline Float4 Float4_Sub(const Float4& v1, const Float4& v2) noexcept
{
	return Float4(v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2], v1[3] - v2[3]);
}

inline Float4 Float4_Multiply(const Float4& v1, const Float4& v2) noexcept
{
	return Float4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

inline Float4 Float4_Divide(const Float4& v1, const Float4& v2) noexcept
{
	return Float4(v1[0] / v2[0], v1[1] / v2[1], v1[2] / v2[2], v1[3] / v2[3]);
}

inline Float3::Float3(const Float4& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
}

inline Float3& Float3::operator=(const Float4& other) noexcept
{
	x = other.x;
	y = other.y;
	z = other.z;

	return *this;
}

//Matrix
struct Matrix4x4
{
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];
	};

	Matrix4x4() { memset(this, 0, sizeof(Matrix4x4)); m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.f; };

	Matrix4x4(const Matrix4x4&) = default;
	Matrix4x4& operator=(const Matrix4x4&) = default;

	Matrix4x4(Matrix4x4&&) = default;
	Matrix4x4& operator=(Matrix4x4&&) = default;

	constexpr Matrix4x4(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33) noexcept
		: _11(m00), _12(m01), _13(m02), _14(m03),
		_21(m10), _22(m11), _23(m12), _24(m13),
		_31(m20), _32(m21), _33(m22), _34(m23),
		_41(m30), _42(m31), _43(m32), _44(m33) {}

	float operator() (size_t Row, size_t Column) const noexcept { return m[Row][Column]; }
	float& operator() (size_t Row, size_t Column) noexcept { return m[Row][Column]; }

	const Float4& GetFloat4(int index) const { return *(Float4*)m[index]; }
	
	Float4& GetFloat4(int index) { return *(Float4*)m[index]; }

	static Matrix4x4 Multiply(const Matrix4x4& M1, const Matrix4x4& M2)
	{
		Matrix4x4 mResult;
		// Cache the invariants in registers
		float x = M1.m[0][0];
		float y = M1.m[0][1];
		float z = M1.m[0][2];
		float w = M1.m[0][3];
		// Perform the operation on the first row
		mResult.m[0][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
		mResult.m[0][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
		mResult.m[0][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
		mResult.m[0][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);
		// Repeat for all the other rows
		x = M1.m[1][0];
		y = M1.m[1][1];
		z = M1.m[1][2];
		w = M1.m[1][3];
		mResult.m[1][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
		mResult.m[1][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
		mResult.m[1][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
		mResult.m[1][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);
		x = M1.m[2][0];
		y = M1.m[2][1];
		z = M1.m[2][2];
		w = M1.m[2][3];
		mResult.m[2][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
		mResult.m[2][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
		mResult.m[2][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
		mResult.m[2][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);
		x = M1.m[3][0];
		y = M1.m[3][1];
		z = M1.m[3][2];
		w = M1.m[3][3];
		mResult.m[3][0] = (M2.m[0][0] * x) + (M2.m[1][0] * y) + (M2.m[2][0] * z) + (M2.m[3][0] * w);
		mResult.m[3][1] = (M2.m[0][1] * x) + (M2.m[1][1] * y) + (M2.m[2][1] * z) + (M2.m[3][1] * w);
		mResult.m[3][2] = (M2.m[0][2] * x) + (M2.m[1][2] * y) + (M2.m[2][2] * z) + (M2.m[3][2] * w);
		mResult.m[3][3] = (M2.m[0][3] * x) + (M2.m[1][3] * y) + (M2.m[2][3] * z) + (M2.m[3][3] * w);
		return mResult;
	}

	Matrix4x4 operator+(const Matrix4x4& other)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m[i][j] += other.m[i][j];
			}
		}

		return *this;
	}

	Matrix4x4 operator-(const Matrix4x4& other)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m[i][j] -= other.m[i][j];
			}
		}

		return *this;
	}

	Matrix4x4 operator*(const Matrix4x4& M) const noexcept
	{
		return Multiply(*this, M);
	}

	Matrix4x4 operator*(float v) const noexcept
	{
		return { m[0][0] * v, m[0][1] * v, m[0][2] * v, m[0][3] * v,
				 m[1][0] * v, m[1][1] * v, m[1][2] * v, m[1][3] * v,
				 m[2][0] * v, m[2][1] * v, m[2][2] * v, m[2][3] * v,
				 m[3][0] * v, m[3][1] * v, m[3][2] * v, m[3][3] * v };
	}

	Matrix4x4& operator+=(const Matrix4x4& other) noexcept
	{
		m[0][0] += other.m[0][0]; m[0][1] += other.m[0][1]; m[0][2] += other.m[0][2]; m[0][3] += other.m[0][3];
		m[1][0] += other.m[1][0]; m[1][1] += other.m[1][1]; m[1][2] += other.m[1][2]; m[1][3] += other.m[1][3];
		m[2][0] += other.m[2][0]; m[2][1] += other.m[2][1]; m[2][2] += other.m[2][2]; m[2][3] += other.m[2][3];
		m[3][0] += other.m[3][0]; m[3][1] += other.m[3][1]; m[3][2] += other.m[3][2]; m[3][3] += other.m[3][3];

		return *this;
	}

	static Matrix4x4 UnitMatrix()
	{
		return {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
	}

	static Matrix4x4 ScaleMatrix(float x, float y, float z)
	{
		return {
			x, 0.0f, 0.0f, 0.0f,
			0.0f, y, 0.0f, 0.0f,
			0.0f, 0.0f, z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
	}

	static Matrix4x4 TranslationMatrix(float x, float y, float z)
	{
		return {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			x, y, z, 1.0f,
		};
	}

	static Matrix4x4 RotationMatrix(float pitch, float yaw, float roll)
	{
		float cp = cosf(pitch);
		float sp = sinf(pitch);

		float cy = cosf(yaw);
		float sy = sinf(yaw);

		float cr = cosf(roll);
		float sr = sinf(roll);

		return {
			cr * cy + sr * sp * sy, sr * cp, sr * sp * cy - cr * sy, 0.0f,
			cr * sp * sy - sr * cy, cr * cp, sr * sy + cr * sp * cy, 0.0f,
			cp * sy, -sp, cp * cy, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
	}
};

//Transform
struct Transform
{
	Float3 Position;
	Float3 Rotation;	//ËÄÔªÊý
	Float3 Scale;

	Transform()
		: Position(0.f, 0.f, 0.f), Rotation(0.f, 0.f, 0.f), Scale(1.f, 1.f, 1.f)
	{}

	operator Matrix4x4() const
	{
		return Matrix4x4::TranslationMatrix(Position.x, Position.y, Position.z) * Matrix4x4::ScaleMatrix(Scale.x, Scale.y, Scale.z)
			* Matrix4x4::RotationMatrix(Rotation.x, Rotation.y, Rotation.z);
	}
};

inline Float4 GetFloat3(const Float3& v)
{
	return Float4(v.x, v.y, v.z, 0.f);
}

inline Float4 GetFloat2(const Float2& v)
{
	return Float4(v.x, v.y, 0.f, 0.f);
}





