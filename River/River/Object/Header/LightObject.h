#pragma once
#include "RiverHead.h"
#include "Object.h"
#include "Component/Header/LightComponent.h"

class LightObject : public Object
{
public:
	LightObject(Float3 direction);

	virtual ~LightObject() override;

	virtual void Tick(float deltaTime);

	const Float3& GetDirection() const { return m_LightComponent->GetDirection(); }

	const Float3& GetLightStrength() const { return m_LightComponent->GetLightStrength(); }

private:
	Share<LightComponent> m_LightComponent;
};
