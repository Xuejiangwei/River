#pragma once
#include "RiverHead.h"
#include "HazeStandardLibraryBase.h"
#include "HazeHeader.h"

class Widget;

class RiverUiLibrary/* : public HazeStandardLibraryBase*/
{
public:
	HAZE_INIT_STANDARD_LIB();

	HAZE_STD_LIB_FUNCTION(void, UiInitialize);

	HAZE_STD_LIB_FUNCTION(Widget*, GetUiByName, const HAZE_STRING* panelName, const HAZE_STRING* widgetName);

	HAZE_STD_LIB_FUNCTION(Widget*, GetWidget, const HAZE_STRING* path);

	HAZE_STD_LIB_FUNCTION(Widget*, GetChildWidget, void* widget, uint64 index);

	HAZE_STD_LIB_FUNCTION(Widget*, GetChildWidgetByName, void* widget, const HAZE_STRING* name);

	HAZE_STD_LIB_FUNCTION(void, SetText, void* widget, const HAZE_STRING* text);

	HAZE_STD_LIB_FUNCTION(void, InfiniteCanvasCreateChildItems, void* widget, int count);
};