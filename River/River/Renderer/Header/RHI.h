#pragma once

#include "RiverHead.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <vector>

struct RHIInitializeParam
{
	int WindowWidth;
	int WindowHeight;
	void* HWnd;
};

enum class APIMode
{
	DX12,
	Vulkan
};

class RHI
{
public:
	RHI();
	virtual ~RHI();

	friend class std::unique_ptr<RHI>;

	virtual void Initialize(const RHIInitializeParam& Param) = 0;

	virtual void Render() = 0;

	virtual void OnUpdate() = 0;

	virtual Share<class PipelineState> BuildPSO(Share<Shader> Shader, const Vector<ShaderLayout>& Layout) = 0;

	virtual Share<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size, uint32_t elementSize, const VertexBufferLayout& layout) = 0;

	virtual Share<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count, ShaderDataType indiceDataType) = 0;

	virtual void Resize(const RHIInitializeParam& param) = 0;

	static Unique<RHI>& Get();

	APIMode GetAPIMode() const { return s_APIMode; }

private:
	static APIMode s_APIMode;

	static Unique<RHI> s_Instance;
};

