#include "RiverPch.h"
#include "GUI/Header/Line.h"
#include "Renderer/Header/RHI.h"

V_Array<Float2> DDALine(const Float2& start, const Float2& end)
{
	V_Array<Float2> points;
	//以下是直线段的绘制（自行补充）
	float x0 = start.x, y0 = start.y, x1 = end.x, y1 = end.y;
	float xbase = x0 + 0.5, ybase = y0 + 0.5;
	float dx = x1 - x0, dy = y1 - y0;
	float length;
	length = (abs(dx) > abs(dy) ? abs(dx) : abs(dy));
	dx /= length; dy /= length;

	points.push_back({ xbase, ybase });
	for (int i = 1; i <= length; i++)
	{
		xbase += dx;
		ybase += dy;
		points.push_back({ xbase, ybase });
	}

	return points;
}

//void MidPointLine(const pair<int, int>& start, const pair<int, int>& end, unsigned long color)
//{
//	CClientDC dc(mView);
//	//以下是直线段的绘制
//	int x1, x2, y1, y2;
//	if (start.first > end.first) x1 = end.first, x2 = start.first, y1 = end.second, y2 = start.second;
//	else x1 = start.first, x2 = end.first, y1 = start.second, y2 = end.second;
//	int a = y1 - y2, b = x2 - x1;
//	int xbase = x1, ybase = y1;
//	int s1 = 1, s2 = (a < 0 ? 1 : -1);
//	if (abs(b) >= abs(a))//dx>dy
//	{
//		int d = ((a << 1) + (a < 0 ? 1 : -1) * b), deta1 = a << 1, deta2 = (a + (a < 0 ? 1 : -1) * b) << 1;
//		while (xbase <= x2)
//		{
//			if ((a < 0) ^ (d >= 0))
//			{
//				xbase += s1, ybase += s2;
//				::SetPixel(hDC, xbase, ybase, color);
//				d += deta2;
//			}
//			else
//			{
//				xbase += s1;
//				::SetPixel(hDC, xbase, ybase, color);
//				d += deta1;
//			}
//		}
//	}
//	else
//	{
//		int d = ((a < 0 ? 1 : -1) * (b << 1)) + a, deta1 = (a < 0 ? 1 : -1) * (b << 1), deta2 = (a + (a < 0 ? 1 : -1) * b) << 1;
//		while (ybase != y2)
//		{
//			if ((a < 0) ^ (d >= 0))
//			{
//				ybase += s2;
//				::SetPixel(hDC, xbase, ybase, color);
//				d += deta1;
//			}
//			else
//			{
//				xbase += s1; ybase += s2;
//				::SetPixel(hDC, xbase, ybase, color);
//				d += deta2;
//			}
//		}
//		::SetPixel(hDC, x2, y2, color);
//	}
//}
//
//void BresenhamLine(const pair<int, int>& start, const pair<int, int>& end, unsigned long color)
//{
//	CClientDC dc(mView); //如果hDC为0时使用
//	//以下是直线段的绘制（自行补充）
//	int x1 = start.first(), y1 = start.second(), x2 = end.first(), y2 = end.second();
//	int dx = x2 - x1, dy = y2 - y1;
//	int s1 = (dx >= 0 ? 1 : -1), s2 = (dy >= 0 ? 1 : -1);
//	dx = abs(dx); dy = abs(dy);
//	dc.SetPixel(x1, y1, color);
//	int xbase = x1, ybase = y1;
//	if (dx >= dy)
//	{
//		int e = (dy << 1) - dx, deta1 = dy << 1, deta2 = (dy - dx) << 1;
//		while (xbase != x2)
//		{
//			if (e >= 0)//y方向增量为1
//				xbase += s1, ybase += s2, e += deta2;
//			else xbase += s1, e += deta1;
//			dc.SetPixel(xbase, ybase, color);
//		}
//
//	}
//	else
//	{
//		int e = (dx << 1) - dy, deta1 = dx << 1, deta2 = (dx - dy) << 1;
//		while (ybase != y2)
//		{
//			if (e >= 0)//x方向增量为1
//				xbase += s1, ybase += s2, e += deta2;
//			else ybase += s2, e += deta1;
//			(!hDC) dc.SetPixel(xbase, ybase, color);
//		}
//	}
//}

Line::Line()
{
	m_Points.clear();
}

Line::~Line()
{
}

void Line::DrawLine(V_Array<UIVertex>& vertices, V_Array<uint16_t>& indices, Byte4 color)
{
	if (m_Points.size() > 1)
	{
		UIRenderItem renderItem;
		renderItem.BaseVertexLocation = (int)vertices.size();
		renderItem.StartIndexLocation = (int)indices.size();
		renderItem.IndexCount = 0;
		renderItem.RenderFlag = (uint32)UIRenderItem::RenderFlag::PointColor;

		for (int i = 0; i < m_Points.size(); i++)
		{
			/*auto points = DDALine(m_Points[i], m_Points[i + 1]);
			for (auto& point : points)
			{
				vertices.push_back(UIVertex(point.x, point.y, 0.0f, color.r, color.g, color.b, color.a));
			}*/
			vertices.push_back(UIVertex(m_Points[i].x, m_Points[i].y, 0.0f, color.r, color.g, color.b, color.a));
			
			/*vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y, 0.0f, 1.0f, 0.0f, color.r, color.g, color.b, color.a));
			vertices.push_back(UIVertex(startPos.x + m_Size.x, startPos.y + m_Size.y, 0.0f, 1.0f, 1.0f, color.r, color.g, color.b, color.a));
			vertices.push_back(UIVertex(startPos.x, startPos.y + m_Size.y, 0.0f, 0.0f, 1.0f, color.r, color.g, color.b, color.a));*/
		}
		
		renderItem.IndexCount += m_Points.size();
		RHI::Get()->AddUIRenderItem(renderItem);
	}
}
