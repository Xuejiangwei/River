#pragma once

#include "RiverHead.h"
#include "RiverTime.h"
#include "Shader.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "RenderItem.h"
#include "Texture.h"
#include "FrameBuffer.h"
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

class StaticMesh;
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

	virtual void BeginFrame() = 0;

	virtual void EndFrame() = 0;

	virtual void Render() = 0;

	virtual void OnUpdate(const RiverTime& time) = 0;

	virtual void UpdateSceneData(const V_Array<RawVertex>& vertices, const V_Array<uint16_t> indices) = 0;

	virtual void UpdateUIData(V_Array<UIVertex>& vertices, V_Array<uint16_t> indices) = 0;

	virtual void SetUpStaticMesh(StaticMesh* mesh) = 0;

	virtual void SetUpMaterial(Material* material) = 0;

	virtual Pair<void*, void*> GetStaticMeshBuffer(const char* name) = 0;

	//virtual Unique<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size, uint32_t elementSize, const VertexBufferLayout& layout) = 0;

	//virtual Unique<IndexBuffer> CreateIndexBuffer(void* indices, uint32_t count, ShaderDataType indiceDataType) = 0;

	virtual void Resize(const RHIInitializeParam& param) = 0;

	virtual Material* CreateMaterial(const char* name) = 0;

	virtual Unique<Texture> CreateTexture(const char* name, const char* path, bool isImmediately = false) = 0;

	virtual class Camera* GetMainCamera() = 0;

	virtual void SetViewPort(uint32 w, uint32 h, uint32 xOffset = 0, uint32 yOffset = 0) = 0;

	virtual void Pick(int x, int y) = 0;

	virtual void GenerateDrawCommands(int commandId, FrameBufferType frameBufferType) = 0;

	virtual void OnSetRenderTargets(int commandId, FrameBufferType frameBufferType) = 0;

	virtual int AllocDrawCommand() = 0;

	virtual void DrawRenderItem(int renderItemId) = 0;

	constexpr int GetRenderItemMaxCount() { return m_RenderItemAllocator.MaxCount; }

	constexpr int GetMaterialMaxCount() { return m_UIRenderItemAllocator.MaxCount; }

	RenderItem* AddRenderItem();

	RenderItem* GetRenderItem(int id) { return &m_RenderItemAllocator.m_Containor[id]; }

	void RemoveRenderItem(int id);
	
	void UpdateRenderItem(int id, RenderItem* renderItem);

	APIMode GetAPIMode() const { return s_APIMode; }

	FontAtlas* GetFont(const char* name = nullptr) const;

	void ClearUIRenderItem() { m_UIRenderItemAllocator.Clear(); }

	void AddUIRenderItem(UIRenderItem& renderItem);

	bool IsShowUIDebugOutline() const { return m_ShowUIDebugOutline; }

	void SetShowUIDebugOutline(bool showOutline) { m_ShowUIDebugOutline = showOutline; }

public:
	static constexpr int GetFrameCount() { return 2; }

	static void SetAPIMode(APIMode mode) { s_APIMode = mode; }

	static Unique<RHI>& Get();

protected:

	template<typename T, typename SizeType = int, int Max = 0>
	class RecycleAllocator
	{
	public:
		using sizeType = SizeType;

		void Clear()
		{
			m_Unuse.clear();
			m_Containor.clear();
		}

		SizeType Alloc()
		{
			SizeType id = -1;
			if (m_Unuse.size() > 0)
			{
				id = m_Unuse.back();
				m_Unuse.resize(m_Unuse.size() - 1);
			}
			else
			{
				if (m_Containor.size() < Max)
				{
					id = (SizeType)m_Containor.size();
					m_Containor.resize(m_Containor.size() + 1);
				}
			}

			return id;
		}

		void Recycle(SizeType index)
		{
			m_Unuse.push_back(index);
		}

	public:
		V_Array<SizeType> m_Unuse;
		V_Array<T> m_Containor;

		const int MaxCount = Max;
	};

protected:
	bool m_ShowUIDebugOutline = false;

	bool m4xMsaaState = false;
	uint32_t m4xMsaaQuality = 0;

	HashMap<String, Unique<FontAtlas>> m_Fonts;

	RecycleAllocator<RenderItem, int, 1000> m_RenderItemAllocator;
	RecycleAllocator<UIRenderItem, int, 2000> m_UIRenderItemAllocator;

private:
	static APIMode s_APIMode;

	static Unique<RHI> s_Instance;
};
