#pragma once
#include "RiverHead.h"
#include "Component.h"
#include "Physics/Header/PhysicsDefine.h"

class BaseMeshComponent : public Component
{
public:
	BaseMeshComponent();

	~BaseMeshComponent();

	void SetColliderEnabled(bool enable) { m_EnableCollider = enable; }

	void SetCollider(Share<CollisionVolume> colliderVolume) { m_ColliderVolume = colliderVolume; }

	CollisionVolume* GetCollisionVolume() const { return m_ColliderVolume.get(); }

	ColliderType GetColliderType() const { return m_ColliderVolume ? m_ColliderVolume->Type : ColliderType::None; }

protected:
	bool m_EnableCollider;
	Share<CollisionVolume> m_ColliderVolume;
};
