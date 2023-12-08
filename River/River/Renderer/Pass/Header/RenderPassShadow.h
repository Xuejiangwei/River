#pragma once

#include "RenderPass.h"

class RenderPassShadow : public RenderPass
{
public:
	RenderPassShadow();

	~RenderPassShadow();

	virtual void Render() override;

private:

};
