#pragma once
#include "RiverHead.h"
#include "Widget.h"

class ListWidget : public Widget
{
public:
	ListWidget();

	~ListWidget();

	virtual void OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices) override;

	void SetChildWidgetType(const char* type);

	template<typename T>
	void SetListData(V_Array<T> data)
	{
		auto parentPos = GetAbsoluteLeftTopPosition();
		for (size_t i = 0; i < 1; i++)
		{
			Share<Widget> widget = nullptr;
			if (m_UnuseChildIndex.size() > 0)
			{
				widget = m_CacheChilds[m_UnuseChildIndex.back()];
				m_UnuseChildIndex.pop_back();
			}
			else
			{
				/*widget = GuiManager::CreateWidgetByTypeName(m_ChildWidgetType.c_str());
				auto text = DynamicCast<Text>(widget);
				text->SetFont(AssetManager::Get()->GetTexture("font"));
				text->SetFontSize(40);
				text->SetText(data[i].c_str());
				text->SetSize(200, 50);
				widget->SetPosition(100, 200);
				m_CacheChilds.push_back(widget);*/
			}
		}
	}

	uint8 GetChildNum() const { return m_ChildNum; }

public:
	static const String& GetWidgetTypeName()
	{
		static String typeName("ListWidget");
		return typeName;
	}

protected:
	V_Array<Share<Widget>> m_CacheChilds;
	V_Array<uint8> m_UnuseChildIndex;
	uint8 m_ChildNum;
	uint8 m_FrontIndex;
	uint8 m_BackIndex;

	bool m_IsDefaultChildWidgetType;
	String m_ChildWidgetType;
};