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
	//2. mtllib �ⲿ���õĲ����ļ�
	//3. o ģ������
	//4. v��ͷ ģ�Ͷ�������
	//5. vt��ͷ uv��ͼ����
	//6. vn��ͷ ÿ����ķ�����
	//7. usemtl ��Ӧ�ⲿ���õĲ����ļ��еĲ���������������ͨ������ָ�����潫ʹ�øò��ʣ����ܻ���ֶ��ѭ��
	//8. s off��ʾ�رչ⻬��
	//9. f��ͷ ��ʾ�ɶ��㡢uv���ꡢ����������ȷ���ı��棬�ж��������ж�����������ʽΪ����������/uv��������/����������
}

void ParseObj::ParseMaterial(const String& path)
{
	//1. comment
	//2. newmtl ��Ӳ�����
	//3. Ns �߹�ɫ��Ȩ��
	//4. Ka �����⣬ֵ�ֱ�ΪRGB
	//5. Kd ������⣬ֵ�ֱ�ΪRGB
	//6. Ks �߹⣬ֵ�ֱ�ΪRGB
	//7. Ke ����⣬ֵ�ֱ�ΪRGB
	//8. Ni ��ѧ�ܶ�
	//9. d ͸���� 0����ȫ͸����1�ǲ���ȫ͸��
	//10. illum ��������ֶ�Ӧָ���Ĺ���ģ��
	
	//����ģ�Ͷ�Ӧ����
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
