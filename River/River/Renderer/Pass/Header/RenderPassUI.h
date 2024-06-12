#pragma once

#include "RenderPass.h"
#include "Renderer/Mesh/Header/Mesh.h"

class Layer;

class RenderPassUI : public RenderPass
{
public:
	RenderPassUI();
	
	virtual ~RenderPassUI() override;

	virtual void Render() override;

	void AddUILayer(Share<Layer>& layer);

	V_Array<UIVertex>& GetRenderVertices() { return m_RenderVertices; }

	V_Array<uint16>& GetRenderIndices() { return m_RenderIndices; }

private:
	V_Array<Share<Layer>> m_Layers;
	V_Array<UIVertex> m_RenderVertices;
	V_Array<uint16> m_RenderIndices;
	RenderPass::PassUniform m_PassUniform;
};
