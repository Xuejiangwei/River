#pragma once
#include "RiverHead.h"
#include "Widget.h"

Share<Widget> DecodeGUI_File(const char* filePath);

Share<Widget> CreateWidget(const char* typeName);

