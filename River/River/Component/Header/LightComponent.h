#pragma once

#include "Component.h"

class LightComponent : public Component
{
public:
	LightComponent();

	virtual ~LightComponent() override;

	virtual void Tick(float deltaTime) override;



private:
	FLOAT_3 m_Position;
	FLOAT_3 m_Rotation;
};
