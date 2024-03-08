#include "RiverPch.h"
#include "GUI/Header/GuiDecode.h"
#include "XJson/XJson.h"
#include "GUI/Header/Panel.h"
#include "GUI/Header/Text.h"
#include "GUI/Header/Image.h"
#include "GUI/Header/Button.h"

Share<Widget> DecodeGUI_File(const char* filePath)
{
	XJson json;
	json.DecodeFromFile(filePath);
	
	auto& w = json["Root"];
	auto type = w["Type"].NodeDataString();
	auto rootPanel = CreateWidget(type);

	auto name = w["Name"].NodeDataString();
	auto size = w["Size"][0].StringToInt32();
	auto pos = w["Position"][0].StringToInt32();

	return rootPanel;
}

Share<Widget> CreateWidget(const char* typeName)
{
	if (typeName == Panel::GetWidgetTypeName())
	{
		return MakeShare<Panel>();
	}
	else if (typeName == Text::GetWidgetTypeName())
	{
		return MakeShare<Text>();
	}
	else if (typeName == Image::GetWidgetTypeName())
	{
		return MakeShare<Image>();
	}
	else if (typeName == Button::GetWidgetTypeName())
	{
		return MakeShare<Button>();
	}

	return nullptr;
}
