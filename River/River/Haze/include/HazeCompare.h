#pragma once

#include "HazeDefine.h"

enum class HazeToken : uint32;
enum class InstructionOpCode : uint32;

enum class HazeCmpType : uint32
{
	None,
	Equal,
	NotEqual,
	Greater,
	GreaterEqual,
	Less,
	LessEqual,

	And,
	Or,
};

enum class HazeOperatorAssign : uint32
{
	None,
	AddAssign,
	SubAssign,
	MulAssign,
	DivAssign,
	ModAssign,

	BitAndAssign,
	BitOrAssign,
	BitXorAssign,

	ShlAssign,
	ShrAssign
};

HazeCmpType GetHazeCmpTypeByToken(HazeToken token);

InstructionOpCode GetInstructionOpCodeByCmpType(HazeCmpType type);

HazeOperatorAssign GetHazeOperatorAssignTypeByToken(HazeToken token);