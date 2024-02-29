#pragma once

enum class ColliderType : unsigned char
{
	None,
	Box,
	Plane,
	Sphere,
};

struct ColliderBase
{
	ColliderBase()
		: Friction(0.4f), Bounciness(0.0f)
	{}

	float Friction;
	float Bounciness;

};

struct ColliderBox : public ColliderBase
{
	Float3 HalfSize = { 0.0f, 0.0f, 0.0f };

public:
	ColliderBox(const Float3& halfSize)
		: HalfSize(halfSize)
	{}
};

struct ColliderPlane : public ColliderBase
{
	Float3 Normal;
	float Distance;

public:
	ColliderPlane(const Float3& normal, float distance)
		: Normal(normal), Distance(distance)
	{}
};

struct ColliderSphere : public ColliderBase
{
	float Radius = 0.0f;
};

struct CollisionVolume
{
	CollisionVolume(ColliderType type, ColliderBase* collider)
		: Type(type), Collider(collider)
	{}

	ColliderType Type = ColliderType::None;

	union
	{
		ColliderBase* Collider;
		ColliderBox* Box;
		ColliderPlane* Plane;
		ColliderSphere* Sphere;
	};

public:
	~CollisionVolume()
	{
		switch (Type)
		{
		case ColliderType::Box:
			delete Box;
			break;
		case ColliderType::Plane:
			delete Plane;
			break;
		case ColliderType::Sphere:
			delete Sphere;
			break;
		case ColliderType::None:
		default:
			if (Collider)
			{
				delete Collider;
			}
			break;
		}
	}
};