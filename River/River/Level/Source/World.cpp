#include "RiverPch.h"
#include "Level/Header/World.h"
#include "Object/Header/Object.h"

#include "Physics/Header/PhyScene.h"

World::World()
{
	m_PhyScene = MakeUnique<PhyScene>();
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

	m_PhyScene->OnUpdate(time.DeltaTime());
}

void World::AddObject(Share<Object>& object)
{
	m_ObjectList.push_back(object);
}
