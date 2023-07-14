#pragma once

#include "RiverHead.h"
#include "Shader.h"
#include "VertexBuffer.h"

#include <vector>

struct RHIInitializeParam
{
	int WindowWidth;
	int WindowHeight;
	void* HWnd;
};

class RHI
{
public:
	RHI();
	virtual ~RHI();
	
	virtual void Initialize(const RHIInitializeParam& Param) = 0;

	virtual Share<class PipelineState> BuildPSO(Share<Shader> Shader, const Vector<ShaderLayout>& Layout) = 0;

	virtual Share<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size, const VertexBufferLayout& layout) = 0;

	virtual void Resize() = 0;

	static Unique<RHI>& Get();
private:
	
	static Unique<RHI> s_Instance;
};

