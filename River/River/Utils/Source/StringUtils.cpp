#include "RiverPch.h"
#include "Utils/Header/StringUtils.h"

#include <windows.h>

std::string WS_2_S(std::wstring wstr)
{
	// support chinese
	std::string res;
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	if (len <= 0) {
		return res;
	}
	char* buffer = new char[len + 1];
	if (buffer == nullptr) {
		return res;
	}
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), buffer, len, nullptr, nullptr);
	buffer[len] = '\0';
	res.append(buffer);
	delete[] buffer;
	return res;
}

std::wstring S_2_WS(std::string str)
{
	std::wstring res;
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), nullptr, 0);
	if (len < 0) {
		return res;
	}
	wchar_t* buffer = new wchar_t[len + 1];
	if (buffer == nullptr) {
		return res;
	}
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), buffer, len);
	buffer[len] = '\0';
	res.append(buffer);
	delete[] buffer;

	return res;
}

V_Array<String> Split(const String& str, const String& delimiter)
{
	V_Array<String> result;

	char* s = new char[str.size() + 1];
	s[str.size()] = '\0';
	strcpy_s(s, str.size() + 1, str.c_str());
	char* p = nullptr;
	char* token = strtok_s(s, delimiter.c_str(), &p);
	while (token)
	{
		result.push_back(token);
		token = strtok_s(NULL, delimiter.c_str(), &p);
	}

	return result;
}
