#pragma once

#include "RiverHead.h"
#include "ShaderDataType.h"

class IndexBuffer
{
public:
	IndexBuffer(uint32 count, ShaderDataType indiceDataType);
	
	virtual ~IndexBuffer();

	virtual void UpdateData(void* context, void* cmdList, void* indices, uint32 count, uint32 additionalCount = 0) {}

	uint32 GetIndexCount() const { return m_Count; }

protected:
	uint32 m_Count;
	ShaderDataType m_IndiceDataType;
};
