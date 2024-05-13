#pragma once
#include "RiverHead.h"
#include <stdio.h>

class RiverFile
{
public:
	RiverFile();

	~RiverFile();

	static void Initialize();

	static FILE* W_OpenFile(const char* path, const char* mode);

	static uint64 FileGetSize(FILE* f);

	static const String& GetRootPath() { return s_RootFilePath; }

	static String GetPathAddRootPath(const char* relativePath);

private:
	static String s_RootFilePath;
};
