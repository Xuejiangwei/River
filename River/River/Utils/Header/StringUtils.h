#pragma once

#include <string>

std::string WS_2_S(std::wstring wstr);

std::wstring S_2_WS(std::string str);

V_Array<String> Split(const String& str, const String& delimiter);