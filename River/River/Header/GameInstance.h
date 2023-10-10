#pragma once
#include "RiverHead.h"

class World;

class GameInstance
{
public:
	GameInstance();

	~GameInstance();

	void SetGameWorld(const String& assetpath);

	World* GetWorld() const { return m_CurrentWorld.get(); }

private:
	Unique<World> m_CurrentWorld;
};
