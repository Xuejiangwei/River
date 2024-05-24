#include "RiverPch.h"
#include "RiverFile.h"

//https://zhuanlan.zhihu.com/p/397397536
//https://vivaxyblog.github.io/2019/12/07/decode-a-png-image-with-javascript-cn.html
struct PNG_IHDR
{
	int Length;
	char TextType[4];

};

void LoadPNG(const char* path, uint8* data);

void LoadPNG(const char* path, uint8* data)
{
	RiverFile file(path, RiverFile::IOStreamMode::Binary);
	auto& stream = file.GetStream();
	//89 50 4E 47 0D 0A 1A 0A
	
	int head[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	for (int i = 0; i < _countof(head); i++)
	{
		int a = stream.get();
		if (head[i] != a)
		{
			LOG("png error %d", a);
		}
	}

	
}