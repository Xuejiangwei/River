#include "RiverPch.h"
#include "RiverFile.h"
#include "Utils/Header/StringUtils.h"

#include <filesystem>

#ifdef _WIN32
	#include <Windows.h>
#endif // DEBUG


String RiverFile::s_RootFilePath = "";
String RiverFile::s_ApplicationFilePath = "";

RiverFile::RiverFile(const char* path, int mode)
{
	m_FileStream.open(path, mode);
}

RiverFile::~RiverFile()
{
	if (m_FileStream.is_open())
	{
		m_FileStream.close();
	}
}

void RiverFile::Initialize()
{
	auto path = std::filesystem::current_path();
	s_ApplicationFilePath = WS_2_S(path);
	s_RootFilePath = WS_2_S(path.parent_path());
}

FILE* RiverFile::W_OpenFile(const char* path, const char* mode)
{
#ifdef _WIN32
	const int filename_wsize = ::MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
	const int mode_wsize = ::MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);
	V_Array<wchar_t> buf;
	buf.resize(filename_wsize + mode_wsize);
	::MultiByteToWideChar(CP_UTF8, 0, path, -1, (wchar_t*)&buf[0], filename_wsize);
	::MultiByteToWideChar(CP_UTF8, 0, mode, -1, (wchar_t*)&buf[filename_wsize], mode_wsize);
	return ::_wfopen((const wchar_t*)&buf[0], (const wchar_t*)&buf[filename_wsize]);
#else
	return nullptr;
#endif
}

uint64 RiverFile::FileGetSize(FILE* f)
{
	long off = 0;
	uint64 sz = 0;
	return ((off = ftell(f)) != -1 && !fseek(f, 0, SEEK_END) && (sz = ftell(f)) != -1 &&
		!fseek(f, off, SEEK_SET)) ? sz : -1;
}

String RiverFile::GetPathAddRootPath(const String& relativePath)
{
	return RiverFile::GetRootPath() + "\\" + relativePath;
}

String RiverFile::GetPathAddApplicationPath(const String& relativePath)
{
	return RiverFile::GetApplicationPath() + "\\" + relativePath;
}

void RiverFile::ReadNumber(char* dst, uint64 size)
{
	m_FileStream.read(dst, size);

	auto end = size - 1;
	uint8 tmp;
	for (int i = 0; i < size / 2; i++)
	{
		tmp = dst[i];
		dst[i] = dst[end - i];
		dst[end - i] = tmp;
	}
}
