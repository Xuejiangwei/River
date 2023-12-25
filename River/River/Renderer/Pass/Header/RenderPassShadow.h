#pragma once

#include "RenderPass.h"

class RenderPassShadow : public RenderPass
{
public:
	RenderPassShadow();

	virtual ~RenderPassShadow() override;

	virtual void Render() override;

private:

};
