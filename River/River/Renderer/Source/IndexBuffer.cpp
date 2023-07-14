#include "RiverPch.h"
#include "IndexBuffer.h"

IndexBuffer::IndexBuffer()
{
}

IndexBuffer::~IndexBuffer()
{
}

Share<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
{
	return Share<IndexBuffer>();
}
