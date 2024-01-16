#pragma once

#include "RenderPass.h"

class RenderPassForwardRendering : public RenderPass
{
	using RenderItemIdType = int;
public:
	RenderPassForwardRendering();

	virtual ~RenderPassForwardRendering() override;

	virtual void Render() override;

private:
	HashMap<int, V_Array<int>> m_RenderBatch;
};

