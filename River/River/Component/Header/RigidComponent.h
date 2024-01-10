#pragma once
#include "Component.h"

class RigidBody;

class RigidComponent : public Component
{
public:
	RigidComponent();

	~RigidComponent();

	RigidBody* GetRigidBody() { return m_RigidBody.get(); }

private:
	Unique<RigidBody> m_RigidBody;
};
