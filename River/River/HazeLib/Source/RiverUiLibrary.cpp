#include "RiverPch.h"
#include "Haze/include/HazeLibraryDefine.h"
#include "Haze/include/HazeStack.h"
#include "Haze/include/HazeVM.h"
#include "HazeLib/Header/RiverUiLibrary.h"

static HashMap<HAZE_STRING, void(*)(HAZE_STD_CALL_PARAM)> s_HashMap_Functions =
{
		{ HAZE_TEXT("通过名字获得UI") , &RiverUiLibrary::GetUiByName },
		{ HAZE_TEXT("设置文本"), &RiverUiLibrary::SetText },
		{ HAZE_TEXT("测试加法"), &RiverUiLibrary::TestAdd },
};

static bool Z_NoUse_RiverUiLibrary = HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("RiverUiLibrary"), &s_HashMap_Functions);

void RiverUiLibrary::InitializeLib()
{
	HazeStandardLibraryBase::AddStdLib(HAZE_TEXT("RiverUiLibrary"), &s_HashMap_Functions);
}

void RiverUiLibrary::GetUiByName(HAZE_STD_CALL_PARAM)
{
	void* widget = nullptr;
	SET_RET(widget, stack);
}

void RiverUiLibrary::GetUiByNameCall(const HAZE_CHAR* name)
{
	
}

void RiverUiLibrary::SetText(HAZE_STD_CALL_PARAM)
{
	SetTextCall(nullptr, nullptr);
}

void RiverUiLibrary::SetTextCall(void* widget, const HAZE_CHAR* name)
{
	
}

void RiverUiLibrary::TestAdd(HAZE_STD_CALL_PARAM)
{
	auto address = stack->GetAddressByESP(HAZE_ADDRESS_SIZE);
	int a, b;

	GET_PARAM_START();
	GET_PARAM(b, address);
	GET_PARAM(a, address);
	
	int c = TestAddCall(a, b);
	SET_RET_BY_TYPE(HazeValueType::Int, c);
}

int RiverUiLibrary::TestAddCall(int a, int b)
{
	return a + b;
}