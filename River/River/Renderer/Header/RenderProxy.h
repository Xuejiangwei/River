#pragma once
#include "RiverHead.h"

class Object;

class RenderProxy
{
	friend class RenderScene;
public:
	RenderProxy(Object* object);

	~RenderProxy();

private:
	Object* m_RenderObject;
};

