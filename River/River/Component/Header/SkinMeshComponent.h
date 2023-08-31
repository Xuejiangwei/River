#pragma once

#include "Component.h"

class SkinMeshComponent : public Component
{
public:
	SkinMeshComponent();

	virtual ~SkinMeshComponent() override;

	virtual void Tick(float deltaTime) override;

private:

};
