#include "RiverPch.h"
#include "GUI/Header/GuiDecode.h"
#include "XJson/XJson.h"
#include "GUI/Header/Panel.h"
#include "GUI/Header/Text.h"
#include "GUI/Header/Image.h"
#include "GUI/Header/Button.h"

static void InitWidgetSetting(Share<Widget>& widget, XJson& json);
static Share<Widget> CreateWidgetByJson(XJson& json);

Share<Widget> DecodeGUI_File(const char* filePath)
{
	XJson json;
	json.DecodeFromFile(filePath);
	
	return CreateWidgetByJson(json["Root"]);
}

Share<Widget> CreateWidgetByJson(XJson& json)
{
	Share<Widget> widget = nullptr;
	auto typeName = json["Type"].Data();

	if (typeName == Panel::GetWidgetTypeName())
	{
		widget = MakeShare<Panel>();
	}
	else if (typeName == Text::GetWidgetTypeName())
	{
		widget = MakeShare<Text>();
	}
	else if (typeName == Image::GetWidgetTypeName())
	{
		widget = MakeShare<Image>();
	}
	else if (typeName == Button::GetWidgetTypeName())
	{
		widget = MakeShare<Button>();
	}

	if (widget)
	{
		InitWidgetSetting(widget, json);

		if (!json["ChildWidgets"].Empty() && typeName == Panel::GetWidgetTypeName())
		{
			for (size_t i = 0; i < json["ChildWidgets"].Size(); i++)
			{
				(*((Panel*)(widget.get())))[CreateWidgetByJson(json["ChildWidgets"][i])];
			}
		}
	}
	
	return widget;
}

void InitWidgetSetting(Share<Widget>& widget, XJson& json)
{
	widget->SetWidgetName(json["Name"].Data());
	widget->SetSize(json["Size"][0].StringToInt32(), json["Size"][1].StringToInt32());
	widget->SetPosition(json["Position"][0].StringToInt32(), json["Position"][1].StringToInt32());
}