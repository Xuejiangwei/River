#include "RiverPch.h"
#include "HazeLibraryDefine.h"
#include "HazeStack.h"
#include "HazeVM.h"
#include "HazeLib/Header/RiverUiLibrary.h"
#include "Application.h"
#include "UILayer.h"

#include "GUI/Header/Text.h"
#include "GUI/Header/InfiniteCanvas.h"
#include "GUI/Header/Panel.h"

static HashMap<HString, void(*)(HAZE_STD_CALL_PARAM)> s_HashMap_Functions =
{
	{ HAZE_TEXT("UI初始化") , &RiverUiLibrary::UiInitialize },
	{ HAZE_TEXT("通过名字获得UI") , &RiverUiLibrary::GetUiByName },
	{ HAZE_TEXT("获得控件") , &RiverUiLibrary::GetWidget },
	{ HAZE_TEXT("获得子控件") , &RiverUiLibrary::GetChildWidget },
	{ HAZE_TEXT("通过名字获得子控件") , &RiverUiLibrary::GetChildWidgetByName },
	{ HAZE_TEXT("设置控件文本"), &RiverUiLibrary::SetText },

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
	HChar* panelName;
	HChar* widgetName;

	GET_PARAM_START();
	GET_PARAM(panelName, address);
	GET_PARAM(widgetName, address);

	auto widget = GetUiByNameCall(panelName, widgetName);
	SET_RET_BY_TYPE(HazeValueType::UnsignedLong, widget);
}

Widget* RiverUiLibrary::GetUiByNameCall(const HChar* panelName, const HChar* widgetName)
{
	return GuiManager::Get()->GetUiWidgetByName(WString2String(panelName).c_str(), WString2String(widgetName).c_str());
}

void RiverUiLibrary::GetWidget(HAZE_STD_CALL_PARAM)
{
	auto address = stack->GetAddressByESP(HAZE_ADDRESS_SIZE);
	HChar* path;

	GET_PARAM_START();
	GET_PARAM(path, address);

	auto widget = GetWidgetCall(path);
	SET_RET_BY_TYPE(HazeValueType::UnsignedLong, widget);
}

Widget* RiverUiLibrary::GetWidgetCall(const HChar* path)
{
	return GuiManager::Get()->GetUiWidgetByPath(WString2String(path));
}

void RiverUiLibrary::GetChildWidget(HAZE_STD_CALL_PARAM)
{
	auto address = stack->GetAddressByESP(HAZE_ADDRESS_SIZE);
	int* panel;
	uint64 index;

	GET_PARAM_START();
	GET_PARAM(panel, address);
	GET_PARAM(index, address);

	auto widget = GetChildWidgetCall(panel, index);
	SET_RET_BY_TYPE(HazeValueType::PointerBase, widget);
}

Widget* RiverUiLibrary::GetChildWidgetCall(void* widget, uint64 index)
{
	return ((Widget*)widget)->GetChildWidget((int)index);
}

void RiverUiLibrary::SetText(HAZE_STD_CALL_PARAM)
{
	auto address = stack->GetAddressByESP(HAZE_ADDRESS_SIZE);
	int* widget;
	HChar* text;

	GET_PARAM_START();
	GET_PARAM(widget, address);
	GET_PARAM(text, address);

	SetTextCall(widget, text);
}

void RiverUiLibrary::SetTextCall(void* widget, const HChar* text)
{
	Text* textWidget = (Text*)widget;

	textWidget->SetText(GB2312_2_UFT8(WString2String(text).c_str()));
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

void RiverUiLibrary::GetChildWidgetByName(HAZE_STD_CALL_PARAM)
{
	auto address = stack->GetAddressByESP(HAZE_ADDRESS_SIZE);
	int* panel;
	HChar* name;

	GET_PARAM_START();
	GET_PARAM(panel);
	GET_PARAM(name);

	auto widget = GetChildWidgetByNameCall(panel, name);
	SET_RET_BY_TYPE(HazeValueType::PointerBase, widget);
}


Widget* RiverUiLibrary::GetChildWidgetByNameCall(void* widget, const HChar* name)
{
	Panel* panel = (Panel*)widget;
	if (panel)
	{
		return panel->GetChildWidgetByName(WString2String(name).c_str());
	}

	return nullptr;
}