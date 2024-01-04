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
	HashMap<int, V_Array<int>> m_RenderBatch;
};
