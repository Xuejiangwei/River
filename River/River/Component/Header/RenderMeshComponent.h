#pragma once
#include "Component.h"
#include "Renderer/Header/RawPolyhedron.h"

class RenderMeshComponent : public Component
{
public:
	RenderMeshComponent();

	virtual ~RenderMeshComponent() override;

	virtual void Tick(float deltaTime) override;

	void SetMeshData(const struct RawPolyhedronData& data);

private:
	V_Array<RawVertex> m_Vertices;
	V_Array<uint16> m_Indices;
};

