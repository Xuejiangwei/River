#pragma once

#include "RiverHead.h"

class RenderPass
{
public:
	RenderPass();

	virtual ~RenderPass();

	virtual void Render() = 0;

private:

};
