#include "RiverPch.h"
#include "Level/Header/World.h"
#include "GameInstance.h"

GameInstance::GameInstance()
{
}

GameInstance::~GameInstance()
{
}

void GameInstance::SetGameWorld(const String& assetPath)
{
	m_CurrentWorld = MakeUnique<World>();
}
