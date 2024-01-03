#pragma once
#include "Component.h"
#include "Renderer/Header/Material.h"

class StaticMesh;

class StaticMeshComponent : public Component
{
public:
	StaticMeshComponent();

	virtual ~StaticMeshComponent() override;

	virtual void Tick(float deltaTime) override;

	StaticMesh* GetStaticMesh() { return m_StaticMesh; }

	void SetStaticMesh(StaticMesh* staticMesh);

	void SetStaticMeshMaterials(V_Array<Material*> materials) { m_Materials = materials; }

	const V_Array<class Material*>& GetMeshMaterials() const { return m_Materials; }

	MaterialBlendMode GetMaterialMode() const;

private:
	StaticMesh* m_StaticMesh;
	V_Array<class Material*> m_Materials;
};