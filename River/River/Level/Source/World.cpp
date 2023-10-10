#include "RiverPch.h"
#include "Level/Header/World.h"
#include "Object/Header/Object.h"

World::World()
{
}

World::~World()
{
}

void World::AddObject(Share<Object>& object)
{
	m_ObjectList.push_back(object);
}
