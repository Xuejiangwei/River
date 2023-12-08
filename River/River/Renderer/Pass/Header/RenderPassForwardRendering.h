#pragma once

#include "RenderPass.h"

class RenderPassForwardRendering : public RenderPass
{
public:
	RenderPassForwardRendering();

	~RenderPassForwardRendering();

	virtual void Render() override;

private:

};

