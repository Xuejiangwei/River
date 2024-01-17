#pragma once

#include "Component.h"

class LightComponent : public Component
{
public:
	LightComponent(Float3 direction);

	virtual ~LightComponent() override;

	virtual void Tick(float deltaTime) override;

	const Float3& GetDirection() const { return m_CurrDirection; }

	const Float3& GetLightStrength() const { return m_LightStrength; }

	void SetLightStrength(const Float3& strength);

private:
	float m_Rotation;
	Float3 m_Position;
	Float3 m_Direction;
	Float3 m_CurrDirection;
	Float3 m_LightStrength;
};
