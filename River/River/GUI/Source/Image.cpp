#include "RiverPch.h"
#include "GUI/Header/Image.h"
#include "Application.h"
#include "Window/Header/Window.h"
#include "Renderer/Header/RHI.h"
#include "Renderer/Texture/Header/Texture.h"

#include "Renderer/DX12Renderer/Header/DX12RHI.h"


Image::Image()
    : m_Texture(nullptr)
{
}

Image::~Image()
{
}

void Image::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
    Widget::OnRender(vertices, indices);

    //if (!RHI::Get()->IsShowUIDebugOutline())
    {
        UIRenderItem renderItem;
        renderItem.RenderTexture = m_Texture;
        renderItem.BaseVertexLocation = (int)vertices.size();
        renderItem.IndexCount = 6;
        renderItem.StartIndexLocation = (int)indices.size();
        
        Float2 startPos = GetAbsoluteLeftTopPosition();

        vertices.push_back(UIVertex(startPos.x, startPos.y, 0.0f, 0.0f, 0.0f, 255, 255, 0, 255));
        vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y, 0.0f, 1.0f, 0.0f, 255, 255, 0, 255));
        vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y + m_Size.y, 0.0f, 1.0f, 1.0f, 255, 255, 0, 255));
        vertices.push_back(UIVertex(startPos.x, startPos.y + m_Size.y, 0.0f, 0.0f, 1.0f, 255, 255, 0, 255));

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(3);

        RHI::Get()->AddUIRenderItem(renderItem);
    }
}

bool Image::OnMouseButtonDown(const MouseButtonPressedEvent& e)
{
    //https://cn.bing.com/search?q=imgui%e5%8a%a0%e8%bd%bd%e5%9b%be%e7%89%87&qs=SC&pq=imgui+%e5%8a%a0%e8%bd%bd&sc=8-8&cvid=B6A3C4F14FBD44F6BBC9E6FEB895B97B&FORM=QBRE&sp=1&lq=0
    //https://blog.csdn.net/zzy1448331580/article/details/104998941
    //https://blog.csdn.net/csdnyonghu123/article/details/102793869

    //auto texture = Texture::CreateTexture("MyImage", "F:\\GitHub\\River\\River\\Textures\\treearray.dds", true);
    
    return true;
}

bool Image::OnMouseButtonRelease(const MouseButtonReleasedEvent& e)
{
    /*auto texture = Texture::CreateTexture("MyImage", "F:\\GitHub\\River\\River\\Textures\\treearray.dds");

    ((DX12RHI*)RHI::Get().get())->AddDescriptor((DX12Texture*)texture);*/
    return true;
}

bool Image::OnMouseMove(int mouseX, int mouseY)
{
    return false;
}

void Image::SetTexture(Texture* texture)
{
	m_Texture = texture;
}