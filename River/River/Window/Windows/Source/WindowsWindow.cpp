#include "RiverPch.h"
#include "../Header/WindowsWindow.h"

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN // 从 Windows 头中排除极少使用的资料
#include <windows.h>
#include <tchar.h>
#include <wrl.h>  //添加WTL支持 方便使用COM
#include <strsafe.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <d3d12.h>	//for d3d12
#include <d3d12shader.h>
#include <d3dcompiler.h>
#if defined(_DEBUG)
#include <dxgidebug.h>
#endif

using namespace Microsoft;
using namespace Microsoft::WRL;
using namespace DirectX;

//linker
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")


#define GRS_WND_CLASS_NAME _T("GRS Game Window Class")
#define GRS_WND_TITLE	_T("GRS DirectX12 Trigger Sample")

#define GRS_THROW_IF_FAILED(hr) {HRESULT _hr = (hr);if (FAILED(_hr)){ throw CGRSCOMException(_hr); }}

class CGRSCOMException
{
public:
	CGRSCOMException(HRESULT hr) : m_hrError(hr)
	{
	}
	HRESULT Error() const
	{
		return m_hrError;
	}
private:
	const HRESULT m_hrError;
};

struct GRS_VERTEX
{
	XMFLOAT4 m_vtPos;
	XMFLOAT4 m_vtColor;
};

LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.
		// We pause the game when the window is deactivated and unpause it
		// when it becomes active.
	case WM_ACTIVATE:
		/*if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.Start();
		}*/
		return 0;

		// WM_SIZE is sent when the user resizes the window.
	case WM_SIZE:
		// Save the new client area dimensions.
		//mClientWidth = LOWORD(lParam);
		//mClientHeight = HIWORD(lParam);
		//if (md3dDevice)
		//{
		//	if (wParam == SIZE_MINIMIZED)
		//	{
		//		mAppPaused = true;
		//		mMinimized = true;
		//		mMaximized = false;
		//	}
		//	else if (wParam == SIZE_MAXIMIZED)
		//	{
		//		mAppPaused = false;
		//		mMinimized = false;
		//		mMaximized = true;
		//		OnResize();
		//	}
		//	else if (wParam == SIZE_RESTORED)
		//	{
		//		// Restoring from minimized state?
		//		if (mMinimized)
		//		{
		//			mAppPaused = false;
		//			mMinimized = false;
		//			OnResize();
		//		}

		//		// Restoring from maximized state?
		//		else if (mMaximized)
		//		{
		//			mAppPaused = false;
		//			mMaximized = false;
		//			OnResize();
		//		}
		//		else if (mResizing)
		//		{
		//			// If user is dragging the resize bars, we do not resize
		//			// the buffers here because as the user continuously
		//			// drags the resize bars, a stream of WM_SIZE messages are
		//			// sent to the window, and it would be pointless (and slow)
		//			// to resize for each WM_SIZE message received from dragging
		//			// the resize bars.  So instead, we reset after the user is
		//			// done resizing the window and releases the resize bars, which
		//			// sends a WM_EXITSIZEMOVE message.
		//		}
		//		else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
		//		{
		//			OnResize();
		//		}
		//	}
		//}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		/*mAppPaused = true;
		mResizing = true;
		mTimer.Stop();*/
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		/*mAppPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();*/
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses
		// a key that does not correspond to any mnemonic or accelerator key.
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		/*((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;*/
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		//OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		//OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYUP:
		/*if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		else if ((int)wParam == VK_F2)
			Set4xMsaaState(!m4xMsaaState);*/
		return 0;
	case WM_KEYDOWN:
		//OnKeyDown(wParam);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return MsgProc(hwnd, msg, wParam, lParam);
}

WindowsWindow::WindowsWindow()
{
}

WindowsWindow::~WindowsWindow()
{

}

void WindowsWindow::Init(const WindowParam& Param)
{
	HINSTANCE hInstance = ::GetModuleHandle(NULL);

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_GLOBALCLASS;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);		//防止无聊的背景重绘
	wc.lpszClassName = WINDOW_CLASS_NAME;
	RegisterClassEx(&wc);

	DWORD dwWndStyle = WS_OVERLAPPED | WS_SYSMENU;
	RECT rtWnd = { 0, 0, Param.WindowWidth, Param.WindowHeight };
	AdjustWindowRect(&rtWnd, dwWndStyle, FALSE);

	RECT R = { 0, 0, Param.WindowWidth, Param.WindowHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	INT posX = (GetSystemMetrics(SM_CXSCREEN) - R.right - R.left) / 2;
	INT posY = (GetSystemMetrics(SM_CYSCREEN) - R.bottom - R.top) / 2;

	auto hWnd = CreateWindowW(WINDOW_CLASS_NAME, L"River窗口", WS_OVERLAPPED | WS_SYSMENU, posX, posY, width, height, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return;
	}

	m_WindowHandle = (void*)hWnd;

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}

void WindowsWindow::OnUpdate()
{

}

bool WindowsWindow::PeekProcessMessage()
{
	if (PeekMessage(&m_Msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&m_Msg);
		DispatchMessage(&m_Msg);

		return true;
	}

	return false;
}
