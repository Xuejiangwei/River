#pragma once

#include "BaseMeshComponent.h"
#include "Renderer/Header/Material.h"

class SkeletalMesh;

class SkeletalMeshComponent : public BaseMeshComponent
{
public:
	SkeletalMeshComponent();

	virtual ~SkeletalMeshComponent() override;

	virtual void Tick(float deltaTime) override;

	SkeletalMesh* GetSkeletalMesh() { return m_SkeletalMesh; }

	void SetSkeletalMesh(SkeletalMesh* skeletalMesh);

	void SetSkeletalMeshMaterials(V_Array<Material*> materials) { m_Materials = materials; }

	const V_Array<class Material*>& GetSkeletalMeshMaterials() const { return m_Materials; }

	MaterialBlendMode GetMaterialMode() const;

private:
	SkeletalMesh* m_SkeletalMesh;
	V_Array<class Material*> m_Materials;
};
