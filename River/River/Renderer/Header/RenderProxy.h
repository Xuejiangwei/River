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

	void GetRenderData(RenderItem* renderItem);

	bool HasRenderData() const;

	bool IsDirty() const { return m_IsDirty; }

	bool HasRenderItem() const { return m_RenderItemId >= 0; }

	void MarkDirty() { m_IsDirty = true; }

	int GetRenderProxyId() const { return m_ProxyId; }

	int GetRenderItemId() const { return m_RenderItemId; }

	MaterialBlendMode GetRenderBlendMode() const;

private:
	Object* m_RenderObject;
	
	int m_RenderItemId;
	int m_ProxyId;
	bool m_IsDirty;
};

