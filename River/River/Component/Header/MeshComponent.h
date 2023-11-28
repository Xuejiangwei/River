#pragma once
#include "Component.h"

class Material;
class StaticMesh;

class MeshComponent : public Component
{
public:
	MeshComponent();

	virtual ~MeshComponent() override;

	virtual void Tick(float deltaTime) override;

	void SetStaticMesh(StaticMesh* staticMesh);

	void SetStaticMeshMaterials(V_Array<Material*> materials);

private:
	StaticMesh* m_StaticMesh;
};