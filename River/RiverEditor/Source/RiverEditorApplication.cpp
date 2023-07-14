
//#include "RiverPch.h"
//#include "RiverHead.h"
//#include "Window.h"
//
//int main()
//{
//	Share<Window> win = Window::Get();
//	win->Create({ 720, 720 });
//
//	return 0;
//}

#include "RiverPch.h"
#include "RiverEditorApplication.h"


RiverEditorApplication::RiverEditorApplication()
	: Application()
{
}

RiverEditorApplication::~RiverEditorApplication()
{
}

Application* CreateApplication()
{
	return new RiverEditorApplication();
}
