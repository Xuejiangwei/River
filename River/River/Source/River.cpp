﻿#include "RiverPch.h"
#include "Application.h"

extern Application* CreateApplication();

int main(int argc, char** argv)
{
	auto app = CreateApplication();
	app->Initialize();
	app->Run();
	delete app;
}