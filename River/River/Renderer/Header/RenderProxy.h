#pragma once
#include "RiverHead.h"

enum class MaterialBlendMode;
class Object;
struct RenderItem;

class RenderProxy
{
public:
	RenderProxy(Object* object);

	~RenderProxy();

	Object* GetObject() { return m_RenderObject; }

	bool IsValid() const { return m_RenderObject != nullptr; }

	void GetRenderData();

	bool HasRenderData() const;

	bool IsDirty() const { return m_IsDirty; }

	bool HasRenderItem() const { return m_RenderItemIds.size() > 0; }

	void MarkDirty() { m_IsDirty = true; }

	int GetRenderProxyId() const { return m_ProxyId; }

	const V_Array<int>& GetRenderItemIds() const { return m_RenderItemIds; }

	MaterialBlendMode GetRenderBlendMode() const;

	void AddLightObjectProxy();

	void AddCameraObjectProxy();

	void AddSkyBoxObjectProxy();

private:
	Object* m_RenderObject;
	
	V_Array<int> m_RenderItemIds;
	int m_ProxyId;
	bool m_IsDirty;
};

