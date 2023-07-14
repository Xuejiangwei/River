#include "RiverPch.h"
#include "RHI.h"

#ifdef _WIN32
	#include "Renderer/DX12Renderer/Header/DX12RHI.h"
	#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"
#endif // _WIN32


Unique<RHI> RHI::s_Instance = nullptr;

RHI::RHI()
{
}

RHI::~RHI()
{
}

Unique<RHI>& RHI::Get()
{
	if (!s_Instance)
	{
#ifdef _WIN32
		s_Instance = MakeUnique<DX12RHI>();
#endif // _WIN32

	}

	return s_Instance;
}
