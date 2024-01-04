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

private:
	Share<LightComponent> m_LightComponent;
};
