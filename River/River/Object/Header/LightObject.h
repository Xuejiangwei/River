#pragma once
#include "RiverHead.h"
#include "Object.h"
#include "Component/Header/LightComponent.h"

class LightObject : public Object
{
public:
	LightObject();

	~LightObject();

	virtual void Tick(float deltaTime);

	const Float3& GetDirection() const { return m_LightComponent->GetDirection(); }

private:
	Share<LightComponent> m_LightComponent;
};
