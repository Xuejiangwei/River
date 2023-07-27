#pragma once

#include "RiverHead.h"
#include "RiverTime.h"
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

enum class RenderLayer
{
	Opaque,
	Mirrors,
	Reflected,
	Transparent,
	Shadow,

	LayerCount
};

class RHI
{
public:
	RHI();

	virtual ~RHI();

	friend class std::unique_ptr<RHI>;

	virtual void Initialize(const RHIInitializeParam& Param) = 0;

	virtual void Render() = 0;

	virtual void OnUpdate(const RiverTime& time) = 0;

	virtual Unique<class PipelineState> BuildPSO(Share<Shader> vsShader, Share<Shader> psShader, const V_Array<ShaderLayout>& Layout) = 0;

	virtual Unique<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size, uint32_t elementSize, const VertexBufferLayout& layout) = 0;

	virtual Unique<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count, ShaderDataType indiceDataType) = 0;

	virtual void Resize(const RHIInitializeParam& param) = 0;

	virtual class Camera* GetMainCamera() = 0;

	static Unique<RHI>& Get();

	APIMode GetAPIMode() const { return s_APIMode; }

protected:
	bool m_4xMsaaState = false;
	unsigned int m_4xMsaaQuality = 0;

private:
	static APIMode s_APIMode;

	static Unique<RHI> s_Instance;
};
