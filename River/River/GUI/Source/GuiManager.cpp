#include "RiverPch.h"
#include "GUI/Header/GuiManager.h"
#include "Application.h"
#include "UILayer.h"

#include "XJson/XJson.h"
#include "GUI/Header/Panel.h"
#include "GUI/Header/Text.h"
#include "GUI/Header/Image.h"
#include "GUI/Header/Button.h"
#include "GUI/Header/Canvas.h"
#include "GUI/Header/ListWidget.h"
#include "GUI/Header/TreeWidget.h"
#include "Renderer/Header/AssetManager.h"

static void InitWidgetSetting(Share<Widget>& widget, XJson& json)
{
	widget->SetWidgetName(json["Name"].Data());
	widget->SetSize(json["Size"][0].StringToInt32(), json["Size"][1].StringToInt32());
	widget->SetPosition(json["Position"][0].StringToInt32(), json["Position"][1].StringToInt32());
}

static Share<Widget> CreateWidgetByJson(XJson& json)
{
	Share<Widget> widget = nullptr;
	auto typeName = json["Type"].Data();

	if (typeName == Panel::GetWidgetTypeName())
	{
		widget = MakeShare<Panel>();
	}
	else if (typeName == Text::GetWidgetTypeName())
	{
		auto text = MakeShare<Text>();
		if (json["Text"].Data())
		{
			text->SetFont(AssetManager::Get()->GetTexture("font"));
			text->SetFontSize(json["FontSize"].StringToInt32());
			text->SetText(json["Text"].Data());
		}

		widget = text;
	}
	else if (typeName == Image::GetWidgetTypeName())
	{
		auto image = MakeShare<Image>();
		if (json["Texture"].Data())
		{
			image->SetTexture(AssetManager::Get()->GetOrCreateTexture(json["Texture"].Data(), json["Texture"].Data()));
		}

		widget = image;
	}
	else if (typeName == Button::GetWidgetTypeName())
	{
		auto button = MakeShare<Button>();
		if (json["Texture"].Data())
		{
			button->SetTexture(AssetManager::Get()->GetOrCreateTexture(json["Texture"].Data(), json["Texture"].Data()));
		}

		widget = button;
	}
	else if (typeName == Canvas::GetWidgetTypeName())
	{
		widget = MakeShare<Canvas>();
	}
	else if (typeName == ListWidget::GetWidgetTypeName())
	{
		auto listWidget = MakeShare<ListWidget>();
		if (json["ChildWidget"].Data())
		{
			listWidget->SetChildWidgetType(json["ChildWidget"].Data());
		}

		widget = listWidget;
	}
	else if (typeName == TreeWidget::GetWidgetTypeName())
	{
		auto treeWidget = MakeShare<TreeWidget>();
		if (json["ChildWidget"].Data())
		{
			treeWidget->SetChildWidgetType(json["ChildWidget"].Data());
		}

		widget = treeWidget;
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

GuiManager::GuiManager()
{
}

GuiManager::~GuiManager()
{
}

Widget* GuiManager::GetUiWidgetByName(const char* panelName, const char* widgetName)
{
	return Application::Get()->GetMainUiLayer()->GetUiByName(panelName, widgetName);
}

Share<Widget> GuiManager::DecodeGUI_File(const char* filePath)
{
	XJson json;
	json.DecodeFromFile(filePath);

	return CreateWidgetByJson(json["Root"]);
}

Share<Widget> GuiManager::CreateWidgetByTypeName(const char* typeName)
{
	Share<Widget> widget = nullptr;
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
	else if (typeName == Canvas::GetWidgetTypeName())
	{
		widget = MakeShare<Canvas>();
	}
	else if (typeName == ListWidget::GetWidgetTypeName())
	{
		widget = MakeShare<ListWidget>();
	}
	else if (typeName == TreeWidget::GetWidgetTypeName())
	{
		widget = MakeShare<TreeWidget>();
	}

	return widget;
}