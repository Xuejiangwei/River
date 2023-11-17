#include "RiverPch.h"
#include "RiverUI/Header/Image.h"
#include "Application.h"
#include "Window/Header/Window.h"
#include "Renderer/Header/RHI.h"

#include "Renderer/DX12Renderer/Header/DX12RHI.h"

#include "Renderer/Header/Texture.h"

Image::Image()
{
}

Image::~Image()
{
}

void Image::OnRender(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices)
{
    Widget::OnRender(vertices, indices);

    UIRenderItem renderItem;
    renderItem.BaseVertexLocation = (int)vertices.size();
    renderItem.IndexCount = 6;
    renderItem.StartIndexLocation = (int)indices.size();

    auto [width, height] = Application::Get().GetWindow()->GetWindowSize();
    FLOAT_2 startPos = GetAbsoluteLeftTopPosition();
    float ndcStartX = startPos.x / width;
    float ndcStartY = -startPos.y / height;
    float ndcLengthX = m_Size.x / width;
    float ndcLengthY = -m_Size.y / height;

    vertices.push_back(UIVertex(ndcStartX, ndcStartY, 0.0f, 0.0f, 0.0f, 255, 255, 0, 255));
    vertices.push_back(UIVertex(ndcStartX + ndcLengthX, ndcStartY, 0.0f, 1.0f, 0.0f, 255, 255, 0, 255));
    vertices.push_back(UIVertex(ndcStartX + ndcLengthX, ndcStartY + ndcLengthY, 0.0f, 1.0f, 1.0f, 255, 255, 0, 255));
    vertices.push_back(UIVertex(ndcStartX, ndcStartY + ndcLengthY, 0.0f, 0.0f, 1.0f, 255, 255, 0, 255));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);

    RHI::Get()->AddUIRenderItem(renderItem);
}

bool Image::OnMouseButtonDown(const Event& e)
{
    /*auto texture = Texture::CreateTexture("MyImage", "F:\\GitHub\\River\\River\\Textures\\treearray.dds");
    ((DX12RHI*)RHI::Get().get())->AddDescriptor(/*(DX12Texture*)texture);*/
    return true;
}

bool Image::OnMouseButtonRelease(const Event& e)
{
    auto texture = Texture::CreateTexture("MyImage", "F:\\GitHub\\River\\River\\Textures\\treearray.dds");

    ((DX12RHI*)RHI::Get().get())->AddDescriptor((DX12Texture*)texture);
    return true;
}

void Image::SetTexture(const char* texturePath)
{
	m_Texture = texturePath;
}