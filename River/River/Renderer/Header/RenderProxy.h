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

	void GetRenderData(RenderItem& renderItem);

	bool HasRenderData() const;

	MaterialBlendMode GetRenderBlendMode() const;

private:
	Object* m_RenderObject;
};

