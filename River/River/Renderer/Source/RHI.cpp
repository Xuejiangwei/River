#include "RiverPch.h"
#include "RHI.h"
#include "Font.h"

#ifdef _WIN32

#include "Renderer/DX12Renderer/Header/DX12RHI.h"
#include "Renderer/DX12Renderer/Header/DX12VertexBuffer.h"

#endif // _WIN32

APIMode RHI::s_APIMode = APIMode::DX12;

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
		switch (s_APIMode)
		{
		case APIMode::DX12:
#ifdef _WIN32
			s_Instance = MakeUnique<DX12RHI>();
#endif // _WIN32
			break;
		case APIMode::Vulkan:
			break;
		default:
			break;
		}
	}

	return s_Instance;
}