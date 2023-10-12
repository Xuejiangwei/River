#pragma once
#include "Component.h"

class StaticMesh;

class MeshComponent : public Component
{
public:
	MeshComponent();

	virtual ~MeshComponent() override;

	virtual void Tick(float deltaTime) override;

	void SetStaticMesh(StaticMesh* staticMesh);

private:
	StaticMesh* m_StaticMesh;
};