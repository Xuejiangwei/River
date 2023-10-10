#pragma once

class Object;

class World
{
	friend Share<Object> ProduceObject();
public:
	World();

	~World();

private:
	void AddObject(Share<Object>& object);

private:
	V_Array<Share<Object>> m_ObjectList;
};

