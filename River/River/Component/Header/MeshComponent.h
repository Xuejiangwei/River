#pragma once
#include "Component.h"

class Material;
class StaticMesh;

class StaticMeshComponent : public Component
{
public:
	StaticMeshComponent();

	virtual ~StaticMeshComponent() override;

	virtual void Tick(float deltaTime) override;

	StaticMesh* GetStaticMesh() { return m_StaticMesh; }

	void SetStaticMesh(StaticMesh* staticMesh);

	void SetStaticMeshMaterials(V_Array<Material*> materials);

private:
	StaticMesh* m_StaticMesh;
};