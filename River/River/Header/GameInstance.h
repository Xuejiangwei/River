#pragma once
#include "RiverHead.h"
#include "RiverTime.h"

class World;

class GameInstance
{
public:
	GameInstance();

	~GameInstance();

	void SetGameWorld(const String& assetpath);

	virtual void OnUpdate(const RiverTime& time);

	World* GetWorld() const { return m_CurrentWorld.get(); }

private:
	Unique<World> m_CurrentWorld;
};
