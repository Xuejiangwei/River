#pragma once
#include "RiverHead.h"
#include "HazeStandardLibraryBase.h"
#include "HazeHeader.h"

class RiverUiLibrary/* : public HazeStandardLibraryBase*/
{
public:
	HAZE_INIT_STANDARD_LIB();

	HAZE_STD_LIB_FUNCTION(void, GetUiByName, const HAZE_CHAR* name);

	HAZE_STD_LIB_FUNCTION(void, SetText, void* widget, const HAZE_CHAR* name);

	HAZE_STD_LIB_FUNCTION(int, TestAdd, int a, int b);
};