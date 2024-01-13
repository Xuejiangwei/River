#pragma once
#include "RiverTime.h"

class Object;
class PhyScene;

class World
{
	template<typename T, typename... _Types>
	friend Share<T> ProduceObject(_Types&& ...);
public:
	World();

	~World();

	void OnUpdate(const RiverTime& time);

private:
	void AddObject(Share<Object> object);

private:
	V_Array<Share<Object>> m_ObjectList;
	Unique<PhyScene> m_PhyScene;
};

