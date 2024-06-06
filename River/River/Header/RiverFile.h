#pragma once
#include "RiverHead.h"
#include <fstream>

class RiverFile
{
public:
	using FileStream = std::ifstream;
	enum IOStreamMode
	{
		In = std::ios_base::in,
		Out = std::ios_base::out,
		Binary = std::ios_base::binary
	};

public:
	RiverFile(const char* path, int mode = IOStreamMode::In);

	~RiverFile();

	static void Initialize();

	static FILE* W_OpenFile(const char* path, const char* mode);

	static uint64 FileGetSize(FILE* f);

	static const String& GetRootPath() { return s_RootFilePath; }

	static const String& GetApplicationPath() { return s_ApplicationFilePath; }

	static String GetPathAddRootPath(const String& relativePath);

	static String GetPathAddApplicationPath(const String& relativePath);

	FileStream& GetStream() { return m_FileStream; }

	void ReadNumber(char* dst, uint64 size);

private:
	static String s_RootFilePath;
	static String s_ApplicationFilePath;

	FileStream m_FileStream;
};
