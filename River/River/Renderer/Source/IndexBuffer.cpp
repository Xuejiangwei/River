#include "RiverPch.h"
#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(uint32_t count, ShaderDataType indiceDataType)
	: m_Count(count), m_IndiceDataType(indiceDataType)
{
}

IndexBuffer::~IndexBuffer()
{
}