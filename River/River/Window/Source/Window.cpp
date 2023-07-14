#include "RiverPCH.h"
#include "Window.h"

#ifdef _WIN32
    #include "../Windows/Header/WindowsWindow.h"
#endif // _WIN32

Window::Window()
{
}

Window::~Window()
{
}

Unique<Window> Window::Create()
{
#ifdef _WIN32
    return MakeUnique<WindowsWindow>();
#endif // _WIN32
}
