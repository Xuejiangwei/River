#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <memory>
#include <cassert>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <list>
#include <array>
#include <functional>

#include "Event.h"
#include "RiverHead.h"
#include "Math/Header/BaseStruct.h"


#if defined(DEBUG) || defined(_DEBUG)
	#define LOG(fmt, ...) std::printf(fmt "\n", __VA_ARGS__) 
#else
	#define LOG(fmt, ...)
#endif