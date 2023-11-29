#pragma once

#include "RiverHead.h"
#include "RiverTime.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderItem.h"
#include "Texture.h"
#include "Renderer/Mesh/Header/Mesh.h"

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

	Highlight,
	Sky,
	Debug,
	SkinnedOpaque,
	LayerCount
};

class Texture;
class Material;
class FontAtlas;

class RHI
{
	friend class std::unique_ptr<RHI>;

public:
	RHI();

	virtual ~RHI();

	virtual void Initialize(const RHIInitializeParam& Param) = 0;

	virtual void Exit() = 0;

	virtual void Render() = 0;

	virtual void OnUpdate(const RiverTime& time) = 0;

	virtual void UpdateSceneData(const V_Array<RawVertex>& vertices, const V_Array<uint16_t> indices) = 0;

	virtual void UpdateUIData(V_Array<UIVertex>& vertices, V_Array<uint16_t> indices) = 0;

	virtual void SetUpStaticMesh(V_Array<Vertex>& vertices, V_Array<uint32>& indices) = 0;

	virtual void SetUpMaterial(Material* material) = 0;

	//virtual Unique<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size, uint32_t elementSize, const VertexBufferLayout& layout) = 0;

	//virtual Unique<IndexBuffer> CreateIndexBuffer(void* indices, uint32_t count, ShaderDataType indiceDataType) = 0;

	virtual void Resize(const RHIInitializeParam& param) = 0;

	virtual Material* CreateMaterial(const char* name) = 0;

	virtual Texture* GetTexture(const char* name) = 0;

	virtual class Camera* GetMainCamera() = 0;

	virtual void Pick(int x, int y) = 0;

	void ClearRenderItem();

	void ClearUIRenderItem();

	void AddRenderItem(RenderItem* renderItem);
	
	void AddUIRenderItem(UIRenderItem& renderItem);

	APIMode GetAPIMode() const { return s_APIMode; }

	FontAtlas* GetFont(const char* name = nullptr) const;
	
	bool IsShowUIDebugOutline() const { return m_ShowUIDebugOutline; }

	void SetShowUIDebugOutline(bool showOutline) { m_ShowUIDebugOutline = showOutline; }

public:
	static Unique<RHI>& Get();

protected:
	bool m_ShowUIDebugOutline = false;

	bool m4xMsaaState = false;
	uint32_t m4xMsaaQuality = 0;
	uint32 m_MaxRenderItemCount = 1000;
	uint32 m_MaxMaterialCount = 1000;

	HashMap<String, Unique<FontAtlas>> m_Fonts;

	V_Array<RenderItem> m_RenderItems;
	V_Array<UIRenderItem> m_UIRenderItems;

private:
	static APIMode s_APIMode;

	static Unique<RHI> s_Instance;
};
