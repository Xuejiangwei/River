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

	HAZE_STD_LIB_FUNCTION(Widget*, GetUiByName, const HChar* panelName, const HChar* widgetName);

	HAZE_STD_LIB_FUNCTION(Widget*, GetWidget, const HChar* path);

	HAZE_STD_LIB_FUNCTION(Widget*, GetChildWidget, void* widget, uint64 index);

	HAZE_STD_LIB_FUNCTION(Widget*, GetChildWidgetByName, void* widget, const HChar* name);

	HAZE_STD_LIB_FUNCTION(void, SetText, void* widget, const HChar* text);

	HAZE_STD_LIB_FUNCTION(void, InfiniteCanvasCreateChildItems, void* widget, int count);
};