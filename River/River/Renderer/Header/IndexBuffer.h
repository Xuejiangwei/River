#pragma once

#include "RiverHead.h"

class IndexBuffer
{
public:
	IndexBuffer();
	virtual ~IndexBuffer();

	static Share<IndexBuffer> Create(uint32_t* indices, uint32_t count);

private:

};
