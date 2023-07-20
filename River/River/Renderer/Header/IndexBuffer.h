#pragma once

#include "RiverHead.h"
#include "ShaderDataType.h"

class IndexBuffer
{
public:
	IndexBuffer(uint32_t count, ShaderDataType indiceDataType);
	virtual ~IndexBuffer();

	uint32_t GetIndexCount() const { return m_Count; }

protected:
	uint32_t m_Count;
	ShaderDataType m_IndiceDataType;
};
