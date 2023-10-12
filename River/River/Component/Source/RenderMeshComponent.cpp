#include "RiverPch.h"
#include "Component/Header/RenderMeshComponent.h"
#include "Renderer/Header/RHI.h"

RenderMeshComponent::RenderMeshComponent()
{
}

RenderMeshComponent::~RenderMeshComponent()
{
}

void RenderMeshComponent::Tick(float deltaTime)
{
	RHI::Get()->UpdateSceneData(m_Vertices, m_Indices);
}

void RenderMeshComponent::SetMeshData(const RawPolyhedronData& data)
{
	m_Vertices = data.Vertices;
	m_Indices = data.Indices;
}