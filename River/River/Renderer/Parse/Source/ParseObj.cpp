#include "RiverPch.h"
#include "Renderer/Parse/Header/ParseObj.h"

ParseObj::ParseObj()
{

}

ParseObj::~ParseObj()
{
}

void ParseObj::Parse(const String& path)
{
	//1. comment
	//2. mtllib 外部引用的材质文件
	//3. o 模型名称
	//4. v开头 模型顶点坐标
	//5. vt开头 uv贴图坐标
	//6. vn开头 每个面的法向量
	//7. usemtl 对应外部引用的材质文件中的材质名，接下来的通过索引指定的面将使用该材质，可能会出现多次循环
	//8. s off表示关闭光滑组
	//9. f开头 表示由顶点、uv坐标、法向量索引确定的表面，有多少面则有多少索引，格式为（顶点索引/uv坐标索引/法向量索引
}

void ParseObj::ParseMaterial(const String& path)
{
	//1. comment
	//2. newmtl 后接材质名
	//3. Ns 高光色的权重
	//4. Ka 环境光，值分别为RGB
	//5. Kd 漫反射光，值分别为RGB
	//6. Ks 高光，值分别为RGB
	//7. Ke 发射光，值分别为RGB
	//8. Ni 光学密度
	//9. d 透明度 0是完全透明，1是不完全透明
	//10. illum 后面的数字对应指定的光照模型
	
	//光照模型对应如下
	//0 Color on and Ambient off
	//1 Color on and Ambient on
	//2 Highlight on
	//3 Reflection on and Ray trace on
	//4 Transparency: Glass on Reflection : Ray trace on
	//5 Reflection : Fresnel on and Ray trace on
	//6 Transparency : Refraction on Reflection : Fresnel off and Ray trace on
	//7 Transparency : Refraction on Reflection : Fresnel on and Ray trace on
	//8 Reflection on and Ray trace off
	//9 Transparency : Glass on Reflection : Ray trace off
	//10 Casts shadows onto invisible surfaces
}
