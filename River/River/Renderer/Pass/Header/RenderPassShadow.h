#pragma once

#include "RenderPass.h"

class RenderPassShadow : public RenderPass
{
public:
	RenderPassShadow();

	virtual ~RenderPassShadow() override;

	virtual void Render() override;

private:
	int m_CommandId;
	Float2 m_ShadowMapSize;
	RenderPass::PassUniform m_PassUniform;
	HashMap<int, V_Array<int>> m_RenderBatch;
};
