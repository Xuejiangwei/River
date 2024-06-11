#include "RiverPch.h"
#include "RiverFile.h"

//https://zhuanlan.zhihu.com/p/397397536
//https://vivaxyblog.github.io/2019/12/07/decode-a-png-image-with-javascript-cn.html

#define USE_LODEPNG 1
#if USE_LODEPNG
	#include "lodepng.h"
#endif // USE_LODEPNG


enum class PNG_INDR_ColorType : uint8
{
	Gray,
	R_G_B = 2,
	Palette,
	Gray_Alpha,

	R_G_B_A = 6,
};

#pragma pack(1)
struct PNG_Chunk_Head
{
	uint32 Length;
	char TextType[4];
};

struct PNG_Chunk
{
	PNG_Chunk_Head Head;
	uint32 CRC;
};

struct PNG_Critical_Chunk_IHDR
{
	uint32 Width;
	uint32 Height;
	uint8 Depth;
	PNG_INDR_ColorType ColorType;
	uint8 Compression;
	uint8 Filter;
	bool Interlace;

	PNG_Chunk BaseData;
};

struct PNG_Critical_Chunk_IDAT
{
	PNG_Chunk BaseData;
	String Data;
};

struct PNG_Ancillary_Chunk_gAMA
{
	PNG_Chunk BaseData;
	uint32 Gamma;
};

struct PNG_Ancillary_Chunk_cHRM
{
	uint32 WhitePointX;
	uint32 WhitePointY;
	uint32 RedX;
	uint32 RedY;
	uint32 GreenX;
	uint32 GreenY;
	uint32 BlueX;
	uint32 BlueY;

	PNG_Chunk BaseData;
};

struct PNG_Ancillary_Chunk_iCCP
{
	PNG_Chunk BaseData;
	String ProfileName; //(0x20空格结尾)
	uint8 Compression;
	String CompressedProfile;
};

struct PNG_Ancillary_Chunk_pHYs
{
	PNG_Chunk BaseData;
	uint32 PixelPerUnitX;	//每单位多少像素，X轴
	uint32 PixelPerUnitY;
	uint8 UnknownUnit;		//0：只定义了像素显示的比例，未定义实际像素的大小   1：表示定义单位为米
};

struct PNG_Ancillary_Chunk_tEXt
{
	PNG_Chunk BaseData;
	String Info;
};

struct PNG_Info
{
	PNG_Critical_Chunk_IHDR IHDR;
	PNG_Critical_Chunk_IDAT IDAT;
	PNG_Chunk IEND;

	Unique<PNG_Ancillary_Chunk_gAMA> gAMA;
	Unique<PNG_Ancillary_Chunk_cHRM> cHRM;
	Unique<PNG_Ancillary_Chunk_iCCP> iCCP;
	Unique<PNG_Ancillary_Chunk_pHYs> pHYs;
	Unique<PNG_Ancillary_Chunk_tEXt> tEXt;
};

//Compression method / flags code : 1 byte
//Additional flags / check bits : 1 byte
//Compressed data blocks : n bytes
//Check value : 4 bytes

#pragma pack()

bool IsPNGTextType(const char* inType, const char* type)
{
	return strlen(inType) >= 4 && strlen(type) >= 4 && inType[0] == type[0] && inType[1] == type[1] && inType[2] == type[2] 
		&& inType[3] == type[3];
}

void LoadPNG(const char* path, V_Array<uint8>& data, uint32& width, uint32& height);

void ReadIHDRChunk(const char*& data, PNG_Info& info);
void ReadIDATChunk(const char*& data, PNG_Info& info);
void ReadIENDChunk(const char*& data, PNG_Info& info);
void ReadPLTEChunk(const char*& data, PNG_Info& info);
void ReadtRNSChunk(const char*& data, PNG_Info& info);
void ReadbKGDChunk(const char*& data, PNG_Info& info);
void ReadtEXtChunk(const char*& data, PNG_Info& info);
void ReadzTXtChunk(const char*& data, PNG_Info& info);
void ReadiTXtChunk(const char*& data, PNG_Info& info);
void ReadtIMEChunk(const char*& data, PNG_Info& info);
void ReadpHYsChunk(const char*& data, PNG_Info& info);
void ReadgAMAChunk(const char*& data, PNG_Info& info);
void ReadcHRMChunk(const char*& data, PNG_Info& info);
void ReadsRGBChunk(const char*& data, PNG_Info& info);
void ReadiCCPChunk(const char*& data, PNG_Info& info);
void ReadsBITChunk(const char*& data, PNG_Info& info);

void LoadPNGChunk(const char* data, PNG_Info& info)
{
	static HashMap<String, void(*)(const char*& data, PNG_Info& info)> s_ReadFunctions = {
		{ "IHDR", &ReadIHDRChunk },
		{ "IDAT", &ReadIDATChunk },
		{ "IEND", &ReadIENDChunk },
		{ "PLTE", &ReadPLTEChunk },
		{ "tRNS", &ReadtRNSChunk },
		{ "bKGD", &ReadbKGDChunk },
		{ "tEXt", &ReadtEXtChunk },
		{ "zTXt", &ReadzTXtChunk },
		{ "iTXt", &ReadiTXtChunk },
		{ "tIME", &ReadtIMEChunk },
		{ "pHYs", &ReadpHYsChunk },
		{ "gAMA", &ReadgAMAChunk },
		{ "cHRM", &ReadcHRMChunk },
		{ "sRGB", &ReadsRGBChunk },
		{ "iCCP", &ReadiCCPChunk },
		{ "sBIT", &ReadsBITChunk },
	};

	while (true)
	{
		auto readFunction = s_ReadFunctions.find({ data[4], data[5], data[6], data[7] });
		if (readFunction != s_ReadFunctions.end())
		{
			readFunction->second(data, info);
		}
		else
		{
			break;
		}
	}
}

unsigned long crc(unsigned char* buf, int len);

void ReadNumber(char* dst, const char* src, uint64 size)
{
	memcpy(dst, src, size);

	auto end = size - 1;
	uint8 tmp;
	for (int i = 0; i < size / 2; i++)
	{
		tmp = dst[i];
		dst[i] = dst[end - i];
		dst[end - i] = tmp;
	}
}

void LoadPNG(const char* path, V_Array<uint8>& data, uint32& width, uint32& height)
{
#if USE_LODEPNG == 0
	RiverFile file(path, RiverFile::IOStreamMode::Binary);
	auto& stream = file.GetStream();
	//89 50 4E 47 0D 0A 1A 0A

	String content(std::istreambuf_iterator<char>(stream), {});
	auto start = content.c_str();

	uint8 head[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	for (int i = 0; i < _countof(head); i++)
	{
		uint8 a = *start++;//stream.get();
		if (head[i] != a)
		{
			LOG("png error %d", a);
		}
	}

	PNG_Info info;
	LoadPNGChunk(start, info);
#else
	unsigned error = lodepng::decode(data, width, height, String(path));
#endif // USE_LODEPNG
}

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
	unsigned long c;
	int n, k;

	for (n = 0; n < 256; n++) {
		c = (unsigned long)n;
		for (k = 0; k < 8; k++) {
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc_table[n] = c;
	}
	crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
   should be initialized to all 1's, and the transmitted value
   is the 1's complement of the final running CRC (see the
   crc() routine below)). */

unsigned long update_crc(unsigned long crc, unsigned char* buf,
	int len)
{
	unsigned long c = crc;
	int n;

	if (!crc_table_computed)
		make_crc_table();
	for (n = 0; n < len; n++) {
		c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
	}
	return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char* buf, int len)
{
	return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}

void ReadIHDRChunk(const char*& data, PNG_Info& info)
{
	uint32 length = 0;
	ReadNumber((char*)(&length), data, sizeof(PNG_Chunk_Head::Length));
	memcpy(&info.IHDR.BaseData.Head.TextType, data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType));
	info.IHDR.BaseData.Head.Length = length;

	auto chunkData = data + sizeof(PNG_Chunk_Head);
	ReadNumber((char*)(&info.IHDR.Width), chunkData, sizeof(info.IHDR.Width));
	ReadNumber((char*)(&info.IHDR.Height), chunkData + offsetof(PNG_Critical_Chunk_IHDR, Height), sizeof(info.IHDR.Height));
	memcpy(&info.IHDR.Depth, chunkData + offsetof(PNG_Critical_Chunk_IHDR, Depth), sizeof(info.IHDR.Depth));
	memcpy(&info.IHDR.ColorType, chunkData + offsetof(PNG_Critical_Chunk_IHDR, ColorType), sizeof(info.IHDR.ColorType));
	memcpy(&info.IHDR.Compression, chunkData + offsetof(PNG_Critical_Chunk_IHDR, Compression), sizeof(info.IHDR.Compression));
	memcpy(&info.IHDR.Filter, chunkData + offsetof(PNG_Critical_Chunk_IHDR, Filter), sizeof(info.IHDR.Filter));
	memcpy(&info.IHDR.Interlace, chunkData + offsetof(PNG_Critical_Chunk_IHDR, Interlace), sizeof(info.IHDR.Interlace));

	ReadNumber((char*)(&info.IHDR.BaseData.CRC), data + sizeof(PNG_Chunk_Head) + length, sizeof(PNG_Chunk::CRC));
	uint32 cc = crc((unsigned char*)data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType) + length);
	if (cc != info.IHDR.BaseData.CRC)
	{
		LOG("png ihdr crc error");
	}

	data += sizeof(PNG_Chunk) + length;
}

void ReadIDATChunk(const char*& data, PNG_Info& info)
{
	uint32 length = 0;
	ReadNumber((char*)(&length), data, sizeof(PNG_Chunk_Head::Length));
	memcpy(&info.IDAT.BaseData.Head.TextType, data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType));
	info.IDAT.BaseData.Head.Length = length;

	info.IDAT.Data.resize(length);
	memcpy(info.IDAT.Data.data(), data + sizeof(PNG_Chunk_Head), length);

	ReadNumber((char*)(&info.IDAT.BaseData.CRC), data + sizeof(PNG_Chunk_Head) + length, sizeof(PNG_Chunk::CRC));
	uint32 cc = crc((unsigned char*)data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType) + length);
	if (cc != info.IDAT.BaseData.CRC)
	{
		LOG("png IDAT crc error");
	}

	data += sizeof(PNG_Chunk) + length;
}

void ReadIENDChunk(const char*& data, PNG_Info& info)
{
	uint32 length = 0;
	ReadNumber((char*)(&length), data, sizeof(PNG_Chunk_Head::Length));
	memcpy(&info.IEND.Head.TextType, data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType));
	info.IEND.Head.Length = length;

	ReadNumber((char*)(&info.IEND.CRC), data + sizeof(PNG_Chunk_Head) + length, sizeof(PNG_Chunk::CRC));
	uint32 cc = crc((unsigned char*)data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType) + length);
	if (cc != info.IEND.CRC)
	{
		LOG("png IEND crc error");
	}

	data += sizeof(PNG_Chunk) + length;
}

void ReadPLTEChunk(const char*& data, PNG_Info& info)
{
}

void ReadtRNSChunk(const char*& data, PNG_Info& info)
{
}

void ReadbKGDChunk(const char*& data, PNG_Info& info)
{
}

void ReadtEXtChunk(const char*& data, PNG_Info& info)
{
	uint32 length = 0;
	auto chunk = MakeUnique<PNG_Ancillary_Chunk_tEXt>();

	ReadNumber((char*)(&length), data, sizeof(PNG_Chunk_Head::Length));
	memcpy(&chunk->BaseData.Head.TextType, data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType));
	chunk->BaseData.Head.Length = length;

	chunk->Info.resize(length);
	memcpy(chunk->Info.data(), data + sizeof(PNG_Chunk_Head), length);

	ReadNumber((char*)(&chunk->BaseData.CRC), data + sizeof(PNG_Chunk_Head) + length, sizeof(PNG_Chunk::CRC));
	uint32 cc = crc((unsigned char*)data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType) + length);
	if (cc != chunk->BaseData.CRC)
	{
		LOG("png tEXt crc error");
	}

	info.tEXt = River::Move(chunk);
	data += sizeof(PNG_Chunk) + length;
}

void ReadzTXtChunk(const char*& data, PNG_Info& info)
{
}

void ReadiTXtChunk(const char*& data, PNG_Info& info)
{
}

void ReadtIMEChunk(const char*& data, PNG_Info& info)
{
}

void ReadpHYsChunk(const char*& data, PNG_Info& info)
{
	uint32 length = 0;
	auto chunk = MakeUnique<PNG_Ancillary_Chunk_pHYs>();

	ReadNumber((char*)(&length), data, sizeof(PNG_Chunk_Head::Length));
	memcpy(&chunk->BaseData.Head.TextType, data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType));
	chunk->BaseData.Head.Length = length;

	auto chunkData = data + sizeof(PNG_Chunk_Head);
	ReadNumber((char*)(&chunk->PixelPerUnitX), chunkData + offsetof(PNG_Ancillary_Chunk_pHYs, PixelPerUnitX), sizeof(chunk->PixelPerUnitX));
	ReadNumber((char*)(&chunk->PixelPerUnitY), chunkData + offsetof(PNG_Ancillary_Chunk_pHYs, PixelPerUnitY), sizeof(chunk->PixelPerUnitY));
	ReadNumber((char*)(&chunk->UnknownUnit), chunkData + offsetof(PNG_Ancillary_Chunk_pHYs, UnknownUnit), sizeof(chunk->UnknownUnit));

	ReadNumber((char*)(&chunk->BaseData.CRC), data + sizeof(PNG_Chunk_Head) + length, sizeof(PNG_Chunk::CRC));
	uint32 cc = crc((unsigned char*)data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType) + length);
	if (cc != chunk->BaseData.CRC)
	{
		LOG("png pHYs crc error");
	}

	info.pHYs = River::Move(chunk);
	data += sizeof(PNG_Chunk) + length;
}

void ReadgAMAChunk(const char*& data, PNG_Info& info)
{
	uint32 length = 0;
	auto chunk = MakeUnique<PNG_Ancillary_Chunk_gAMA>();

	ReadNumber((char*)(&length), data, sizeof(PNG_Chunk_Head::Length));
	memcpy(&chunk->BaseData.Head.TextType, data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType));
	chunk->BaseData.Head.Length = length;

	ReadNumber((char*)(&chunk->Gamma), data + sizeof(PNG_Chunk_Head), sizeof(chunk->Gamma));

	ReadNumber((char*)(&chunk->BaseData.CRC), data + sizeof(PNG_Chunk_Head) + length, sizeof(PNG_Chunk::CRC));
	uint32 cc = crc((unsigned char*)data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType) + length);
	if (cc != chunk->BaseData.CRC)
	{
		LOG("png gAMA crc error");
	}

	info.gAMA = River::Move(chunk);
	data += sizeof(PNG_Chunk) + length;
}

void ReadcHRMChunk(const char*& data, PNG_Info& info)
{
	uint32 length = 0;
	auto chunk = MakeUnique<PNG_Ancillary_Chunk_cHRM>();

	ReadNumber((char*)(&length), data, sizeof(PNG_Chunk_Head::Length));
	memcpy(&chunk->BaseData.Head.TextType, data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType));
	chunk->BaseData.Head.Length = length;

	auto chunkData = data + sizeof(PNG_Chunk_Head);
	ReadNumber((char*)(&chunk->WhitePointX), chunkData, sizeof(chunk->WhitePointX));
	ReadNumber((char*)(&chunk->WhitePointY), chunkData + offsetof(PNG_Ancillary_Chunk_cHRM, WhitePointY), sizeof(chunk->WhitePointY));
	ReadNumber((char*)(&chunk->RedX), chunkData + offsetof(PNG_Ancillary_Chunk_cHRM, RedX), sizeof(chunk->RedX));
	ReadNumber((char*)(&chunk->RedY), chunkData + offsetof(PNG_Ancillary_Chunk_cHRM, RedY), sizeof(chunk->RedY));
	ReadNumber((char*)(&chunk->GreenX), chunkData + offsetof(PNG_Ancillary_Chunk_cHRM, GreenX), sizeof(chunk->GreenX));
	ReadNumber((char*)(&chunk->GreenY), chunkData + offsetof(PNG_Ancillary_Chunk_cHRM, GreenY), sizeof(chunk->GreenY));
	ReadNumber((char*)(&chunk->BlueX), chunkData + offsetof(PNG_Ancillary_Chunk_cHRM, BlueX), sizeof(chunk->BlueX));
	ReadNumber((char*)(&chunk->BlueY), chunkData + offsetof(PNG_Ancillary_Chunk_cHRM, BlueY), sizeof(chunk->BlueY));

	ReadNumber((char*)(&chunk->BaseData.CRC), data + sizeof(PNG_Chunk_Head) + length, sizeof(PNG_Chunk::CRC));
	uint32 cc = crc((unsigned char*)data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType) + length);
	if (cc != chunk->BaseData.CRC)
	{
		LOG("png cHRM crc error");
	}

	info.cHRM = River::Move(chunk);
	data += sizeof(PNG_Chunk) + length;
}

void ReadsRGBChunk(const char*& data, PNG_Info& info)
{
}

void ReadiCCPChunk(const char*& data, PNG_Info& info)
{
	uint32 length = 0;
	auto chunk = MakeUnique<PNG_Ancillary_Chunk_iCCP>();

	ReadNumber((char*)(&length), data, sizeof(PNG_Chunk_Head::Length));
	memcpy(&chunk->BaseData.Head.TextType, data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType));
	chunk->BaseData.Head.Length = length;

	chunk->ProfileName = data + sizeof(PNG_Chunk_Head);
	chunk->Compression = *(data + sizeof(PNG_Chunk_Head) + chunk->ProfileName.length());

	ReadNumber((char*)(&chunk->BaseData.CRC), data + sizeof(PNG_Chunk_Head) + length, sizeof(PNG_Chunk::CRC));
	uint32 cc = crc((unsigned char*)data + sizeof(PNG_Chunk_Head::Length), sizeof(PNG_Chunk_Head::TextType) + length);
	if (cc != chunk->BaseData.CRC)
	{
		LOG("png cHRM crc error");
	}

	info.iCCP = River::Move(chunk);
	data += sizeof(PNG_Chunk) + length;
}

void ReadsBITChunk(const char*& data, PNG_Info& info)
{
}
