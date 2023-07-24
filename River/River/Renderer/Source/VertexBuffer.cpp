#include "RiverPch.h"
#include "RHI.h"
#include "VertexBuffer.h"

#ifdef _WIN32
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#endif // _WIN32

VertexBuffer::VertexBuffer(const VertexBufferLayout& layout)
	: m_Layout(std::move(layout))
{
}

VertexBuffer::~VertexBuffer()
{
}

//Share<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
//{
//#ifdef _WIN32
//	return MakeShare<DX12VertexBuffer>(vertices, size);
//#endif // _WIN32
//
//
//	return Share<VertexBuffer>();
//}