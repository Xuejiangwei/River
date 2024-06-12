#pragma once

#include "RenderPass.h"

class RenderPassForwardRendering : public RenderPass
{
	using RenderItemIdType = int;
public:
	RenderPassForwardRendering();

	virtual ~RenderPassForwardRendering() override;

	virtual void Render() override;

	const RenderPass::PassUniform& GetPassUniform() { return m_PassUniform; }

private:
	HashMap<int, V_Array<int>> m_RenderBatch;
	RenderPass::PassUniform m_PassUniform;
};

