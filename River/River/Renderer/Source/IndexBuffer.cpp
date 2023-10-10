#include "RiverPch.h"
#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(uint32 count, ShaderDataType indiceDataType)
	: m_Count(count), m_IndiceDataType(indiceDataType)
{
}

IndexBuffer::~IndexBuffer()
{
}