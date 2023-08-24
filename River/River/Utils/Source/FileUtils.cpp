#include "RiverPch.h"
#include "Utils/Header/FileUtils.h"

#ifdef _WIN32
	#include <Windows.h>
#endif // DEBUG


FILE* W_OpenFile(const char* path, const char* mode)
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

uint64 FileGetSize(FILE* f)
{
	long off = 0;
	uint64 sz = 0;
	return ((off = ftell(f)) != -1 && !fseek(f, 0, SEEK_END) && (sz = ftell(f)) != -1 && 
		!fseek(f, off, SEEK_SET)) ? sz : -1;
}