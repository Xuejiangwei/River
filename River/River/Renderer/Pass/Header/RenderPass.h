#pragma once

#include "RiverHead.h"

class RenderPass
{
public:
	RenderPass();

	~RenderPass();

	virtual void Render() = 0;

private:

};
