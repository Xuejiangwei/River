#pragma once
#include "RiverTime.h"

class Object;

class World
{
	friend Share<Object> ProduceObject();
public:
	World();

	~World();

	void OnUpdate(const RiverTime& time);

private:
	void AddObject(Share<Object>& object);

private:
	V_Array<Share<Object>> m_ObjectList;
};

