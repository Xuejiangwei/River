#pragma once

#include "Component.h"

class LightComponent : public Component
{
public:
	LightComponent();

	virtual ~LightComponent() override;

	virtual void Tick(float deltaTime) override;

	const Float3& GetDirection() const { return m_CurrDirection; }

private:
	float m_Rotation;
	Float3 m_Position;
	Float3 m_Direction;
	Float3 m_CurrDirection;
};
