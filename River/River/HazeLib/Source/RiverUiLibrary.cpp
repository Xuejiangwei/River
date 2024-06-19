#include "RiverPch.h"
#include "HazeLibraryDefine.h"
#include "HazeStack.h"
#include "HazeVM.h"
#include "HazeLib/Header/RiverUiLibrary.h"
#include "Application.h"
#include "UILayer.h"

#include "GUI/Header/Text.h"
#include "GUI/Header/InfiniteCanvas.h"

static HashMap<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)> s_HashMap_Functions =
{
	{ HAZE_TEXT("UI初始化") , &RiverUiLibrary::UiInitialize },
	{ HAZE_TEXT("通过名字获得UI") , &RiverUiLibrary::GetUiByName },
	{ HAZE_TEXT("获得控件") , &RiverUiLibrary::GetWidget },
	{ HAZE_TEXT("设置控件文本"), &RiverUiLibrary::SetText },
	{ HAZE_TEXT("测试加法"), &RiverUiLibrary::TestAdd },

	{ HAZE_TEXT("无限画布生成子UI"), &RiverUiLibrary::InfiniteCanvasCreateChildItems },
};

static bool Z_NoUse_RiverUiLibrary = HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("RiverUiLibrary"), &s_HashMap_Functions);

void RiverUiLibrary::InitializeLib()
{
	HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("RiverUiLibrary"), &s_HashMap_Functions);
}

void RiverUiLibrary::UiInitialize(HAZE_STD_CALL_PARAM)
{
	UiInitializeCall();
}

void RiverUiLibrary::UiInitializeCall()
{
	Application::Get()->GetMainUiLayer()->OnInitialize();
}

void RiverUiLibrary::GetUiByName(HAZE_STD_CALL_PARAM)
{
	auto address = stack->GetAddressByESP(HAZE_ADDRESS_SIZE);
	HAZE_STRING* panelName;
	HAZE_STRING* widgetName;

	GET_PARAM_START();
	GET_PARAM(panelName, address);
	GET_PARAM(widgetName, address);

	auto widget = GetUiByNameCall(panelName, widgetName);
	uint64 widgetPtr = (uint64)widget;
	SET_RET_BY_TYPE(HazeValueType::UnsignedLong, widget);
}

Widget* RiverUiLibrary::GetUiByNameCall(const HAZE_STRING* panelName, const HAZE_STRING* widgetName)
{
	return GuiManager::Get()->GetUiWidgetByName(WString2String(*panelName).c_str(), WString2String(*widgetName).c_str());
}

void RiverUiLibrary::GetWidget(HAZE_STD_CALL_PARAM)
{
	auto address = stack->GetAddressByESP(HAZE_ADDRESS_SIZE);
	HAZE_STRING* path;

	GET_PARAM_START();
	GET_PARAM(path, address);

	auto widget = GetWidgetCall(path);
	uint64 widgetPtr = (uint64)widget;
	SET_RET_BY_TYPE(HazeValueType::UnsignedLong, widget);
}

Widget* RiverUiLibrary::GetWidgetCall(const HAZE_STRING* path)
{
	return GuiManager::Get()->GetUiWidgetByPath(WString2String(*path));
}

void RiverUiLibrary::SetText(HAZE_STD_CALL_PARAM)
{
	auto address = stack->GetAddressByESP(HAZE_ADDRESS_SIZE);
	int* widget;
	HAZE_STRING* text;

	GET_PARAM_START();
	GET_PARAM(widget, address);
	GET_PARAM(text, address);

	SetTextCall(widget, text);
}

void RiverUiLibrary::SetTextCall(void* widget, const HAZE_STRING* text)
{
	Text* textWidget = (Text*)widget;

	textWidget->SetText(GB2312_2_UFT8(WString2String(*text).c_str()));
}

void RiverUiLibrary::TestAdd(HAZE_STD_CALL_PARAM)
{
	auto address = stack->GetAddressByESP(HAZE_ADDRESS_SIZE);
	int a, b;

	GET_PARAM_START();
	GET_PARAM(b, address);
	GET_PARAM(a, address);
	
	int c = TestAddCall(a, b);
	SET_RET_BY_TYPE(HazeValueType::Int, c);
}

int RiverUiLibrary::TestAddCall(int a, int b)
{
	return a + b;
}

void RiverUiLibrary::InfiniteCanvasCreateChildItems(HAZE_STD_CALL_PARAM)
{
	auto address = stack->GetAddressByESP(HAZE_ADDRESS_SIZE);
	int* widget;
	int count;

	GET_PARAM_START();
	GET_PARAM(widget, address);
	GET_PARAM(count, address);

	InfiniteCanvasCreateChildItemsCall(widget, count);
}

void RiverUiLibrary::InfiniteCanvasCreateChildItemsCall(void* widget, int count)
{
	InfiniteCanvas* canvas = (InfiniteCanvas*)widget;
	if (canvas)
	{
		canvas->CreateChildWidget(count);
	}
}
