#pragma once

#include "Component.h"

class MeshComponent : public Component
{
public:
	MeshComponent();

	virtual ~MeshComponent() override;

	virtual void Tick(float deltaTime);

private:

};