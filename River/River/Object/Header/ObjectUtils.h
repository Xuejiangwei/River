#pragma once
#include "RiverHead.h"
#include "Object.h"
#include "Level/Header/World.h"

template<typename T = Object, typename... _Types>
Share<T> ProduceObject(_Types&&... _Args)
{
	auto obj = MakeShare<T>(std::forward<_Types>(_Args)...);
	auto world = Application::Get()->GetGameInstance()->GetWorld();
	world->AddObject(obj);
	return obj;
}