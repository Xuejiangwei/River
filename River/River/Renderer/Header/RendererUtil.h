#pragma once

class RendererUtil
{
public:
	static unsigned int CalcMinimumGPUAllocSize(unsigned int size)
	{
		return (size + 255) & ~255;
	}
};