#pragma once
#include "RiverHead.h"
#include "Widget.h"

Share<Widget> DecodeGUI_File(const char* filePath);

Share<Widget> CreateWidgetByTypeName(const char* typeName);