#include "RiverPch.h"
#include "Level/Header/World.h"
#include "Object/Header/Object.h"

World::World()
{
}

World::~World()
{
}

void World::OnUpdate(const RiverTime& time)
{
	for (auto& obj : m_ObjectList)
	{
		obj->Tick(time.DeltaTime());
	}
}

void World::AddObject(Share<Object>& object)
{
	m_ObjectList.push_back(object);
}
