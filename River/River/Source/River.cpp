#include "RiverPch.h"
#include "RiverFile.h"
#include "Application.h"

extern Application* CreateApplication();

int main(int argc, char** argv)
{
	RiverFile::Initialize();

	auto app = CreateApplication();
	app->Initialize();
	app->Run();
	delete app;
}