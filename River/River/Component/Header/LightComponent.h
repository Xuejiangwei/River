#pragma once

#include "Component.h"

class LightComponent : public Component
{
public:
	LightComponent();

	virtual ~LightComponent() override;

	virtual void Tick(float deltaTime) override;

private:
	float m_Rotation;
	Float3 m_Position;
	Float3 m_Direction;
	Float3 m_CurrDirection;
};
