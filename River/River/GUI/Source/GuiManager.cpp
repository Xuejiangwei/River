#include "RiverPch.h"
#include "Application.h"
#include "UILayer.h"

#include "XJson/XJson.h"
#include "GUI/Header/Panel.h"
#include "GUI/Header/Text.h"
#include "GUI/Header/Image.h"
#include "GUI/Header/Button.h"
#include "GUI/Header/Canvas.h"
#include "GUI/Header/InfiniteCanvas.h"
#include "GUI/Header/ListWidget.h"
#include "GUI/Header/TreeWidget.h"
#include "Renderer/Header/AssetManager.h"

static void InitWidgetSetting(Share<Widget>& widget, XJson& json)
{
	widget->SetWidgetName(json["Name"].Data());
	widget->SetSize(json["Size"][0].StringToFloat(), json["Size"][1].StringToFloat());
	widget->SetPosition(json["Position"][0].StringToFloat(), json["Position"][1].StringToFloat());
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
			text->SetFontSize(json["FontSize"].StringToFloat());
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
	else if (typeName == InfiniteCanvas::GetWidgetTypeName())
	{
		auto infiniteCanvasWidget = MakeShare<InfiniteCanvas>();
		if (json["ChildWidget"].Data())
		{
			infiniteCanvasWidget->SetChildWidgetType(json["ChildWidget"].Data());
		}

		widget = infiniteCanvasWidget;
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

Share<Widget> GuiManager::DecodeGUI_File(const String& filePath)
{
	XJson json;
	json.DecodeFromFile(filePath.c_str());

	return CreateWidgetByJson(json["Root"]);
}

Share<Widget> GuiManager::CreateWidgetByTypeName(const String& typeName, Widget* parent)
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
	else if (typeName == InfiniteCanvas::GetWidgetTypeName())
	{
		widget = MakeShare<InfiniteCanvas>();
	}
	else if (typeName == ListWidget::GetWidgetTypeName())
	{
		widget = MakeShare<ListWidget>();
	}
	else if (typeName == TreeWidget::GetWidgetTypeName())
	{
		widget = MakeShare<TreeWidget>();
	}
	else if (typeName.find(".json") != std::string::npos)
	{
		widget = DecodeGUI_File(typeName);
	}

	widget->m_Parent = parent;
	return widget;
}