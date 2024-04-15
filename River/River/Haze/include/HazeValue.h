#pragma once

#include "HazeDefine.h"
#include <iostream>

#define CAST_TYPE(V) (uint32)V

enum class HazeValueType : uint32
{
	None,

	Void,
	Bool,

	Byte,
	UnsignedByte,

	Char,

	Short,
	UnsignedShort,

	Int,
	Float,
	Long,
	Double,

	UnsignedInt,
	UnsignedLong,

	ArrayBase,
	ArrayClass,
	ArrayPointer, //��ͨ���͵�ָ�������ָ��ָ���ָ�����鶼��������ͣ�����ָ��ָ���ָ�룬��ֻ��Ҫ������AST->CodeGen�׶��ж��Ƿ���ͬһ����

	PointerBase,
	PointerClass,
	PointerFunction,
	PointerPointer,

	ReferenceBase,
	ReferenceClass,

	Class,
	Function,

	Enum,

	MultiVariable,
};

struct HazeValue
{
	union
	{
		bool Bool;

		hbyte Byte;
		uhbyte UnsignedByte;

		HAZE_CHAR Char;

		short Short;
		ushort UnsignedShort;

		int Int;
		float Float;
		int64 Long;
		double Double;

		uint32 UnsignedInt;
		uint64 UnsignedLong;

		const void* Pointer;

		union
		{
			int StringTableIndex;
			int MemorySize;
			//const HAZE_STRING* StringPointer;
		} Extra;
	} Value;

public:
	HazeValue& operator =(const HazeValue& V)
	{
		memcpy(this, &V, sizeof(V));
		return *this;
	}

	HazeValue& operator =(int64_t V)
	{
		memcpy(&this->Value, &V, sizeof(V));
		return *this;
	}
};

enum class InstructionOpCode : uint32;
enum class HazeToken : uint32;

uint32 GetSizeByHazeType(HazeValueType type);

HazeToken GetTokenByValueType(HazeValueType type);

HazeValueType GetValueTypeByToken(HazeToken token);

HazeValueType GetStrongerType(HazeValueType type1, HazeValueType type2);

bool IsNoneType(HazeValueType type);

bool IsVoidType(HazeValueType type);

bool IsHazeDefaultTypeAndVoid(HazeValueType type);

bool IsHazeDefaultType(HazeValueType type);

bool IsIntegerType(HazeValueType type);

bool IsFloatingType(HazeValueType type);

bool IsUnsignedLongType(HazeValueType type);

bool IsPointerType(HazeValueType type);

bool IsOneLevelPointerType(HazeValueType type);

bool IsPointerFunction(HazeValueType type);

bool IsPointerPointer(HazeValueType type);

bool IsNumberType(HazeValueType type);

bool IsClassType(HazeValueType type);

bool IsArrayType(HazeValueType type);

bool IsArrayPointerType(HazeValueType type);

bool IsReferenceType(HazeValueType type);

void StringToHazeValueNumber(const HAZE_STRING& str, HazeValueType type, HazeValue& value);

void OperatorValueByType(HazeValueType type, InstructionOpCode typeCode, const void* target);

void CalculateValueByType(HazeValueType type, InstructionOpCode typeCode, const void* source, const void* target);

void CompareValueByType(HazeValueType type, struct HazeRegister* hazeRegister, const void* source, const void* target);

size_t GetHazeCharPointerLength(const HAZE_CHAR* hChar);

const HAZE_CHAR* GetHazeValueTypeString(HazeValueType type);

HAZE_BINARY_CHAR* GetBinaryPointer(HazeValueType type, const HazeValue& value);

HazeValue GetNegValue(HazeValueType type, const HazeValue& value);

bool CanCVT(HazeValueType type1, HazeValueType type2);