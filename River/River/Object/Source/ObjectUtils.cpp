#include "RiverPch.h"
#include "Application.h"
#include "GameInstance.h"
#include "Level/Header/World.h"
#include "Object/Header/ObjectUtils.h"

Share<Object> ProduceObject()
{
	auto obj = MakeShare<Object>();
	auto world = Application::Get().GetGameInstance()->GetWorld();
	world->AddObject(obj);
	return obj;
}
