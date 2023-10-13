#pragma once


namespace River
{
	struct RGBA32
	{
		uint8 r, g, b, a;

		RGBA32() : r(0), g(0), b(0), a(255) {}

		RGBA32(uint8 r, uint8 g, uint8 b, uint8 a)
			: r(r), g(g), b(b), a(a) {}

		operator uint32() const { return ((uint32)(r) << 0) | ((uint32)(g) << 8)
			| ((uint32)(g) << 16) | ((uint32)(a) << 24); }
	};

	struct Float2
	{
		float x = 0.f, y = 0.f;

		Float2() {}

		Float2(float x, float y) : x(x), y(y) {}

		Float2 operator+(const Float2& other)
		{
			return Float2(x + other.x, y + other.y);
		}

		Float2 operator*(float other)
		{
			return Float2(x * other, y * other);
		}

		Float2 operator*(Float2 other)
		{
			return Float2(x * other.x, y * other.y);
		}
	};

	inline Float2 operator*(float other, Float2 v) noexcept
	{
		return Float2(v.x * other, v.y * other);
	}

	struct Float3
	{
		float x = 0.f, y = 0.f, z = 0.f;

		Float3() {};

		Float3(float x, float y, float z) : x(x), y(y), z(z) {}

		Float3 operator+(const Float3& other)
		{
			return Float3(x + other.x, y + other.y, z + other.z);
		}

		Float3 Dot(const Float3& other)
		{
			float fValue = x * other.x + y * other.y + z * other.z;
			return { fValue, fValue, fValue };
		}

		Float3 LengthSq()
		{
			return Dot(*this);
		}

		Float3 Sqrt()
		{
			return { x * x, y * y, z * z };
		}

		Float3 Length()
		{
			Float3 Result;

			Result = LengthSq();
			Result = Result.Sqrt();

			return Result;
		}

		Float3 Normalize()
		{
			float fLength;
			Float3 vResult;

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
	};

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

		Matrix4x4() = default;

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
		explicit Matrix4x4(_In_reads_(16) const float* pArray) noexcept;

		float operator() (size_t Row, size_t Column) const noexcept { return m[Row][Column]; }
		float& operator() (size_t Row, size_t Column) noexcept { return m[Row][Column]; }

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

		Matrix4x4 operator*(const Matrix4x4& M) const noexcept
		{
			return Multiply(*this, M);
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
	};

	//Transform
	struct Transform
	{
		Float3 Position;
		Float4 Rotation;	//ËÄÔªÊý
		Float3 Scale;

		Transform()
			: Position(0.f, 0.f, 0.f), Rotation(0.f, 0.f, 0.f, 0.f), Scale(1.f, 1.f, 1.f)
		{}

		operator Matrix4x4() const
		{
			return Matrix4x4::TranslationMatrix(Position.x, Position.y, Position.z) * Matrix4x4::ScaleMatrix(Scale.x, Scale.y, Scale.z);
		}
	};
}

using UINT8_4 = River::RGBA32;
using FLOAT_2 = River::Float2;
using FLOAT_3 = River::Float3;
using FLOAT_4 = River::Float4;

using Matrix_4_4 = River::Matrix4x4;
using Transform = River::Transform;