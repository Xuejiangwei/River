#pragma once
#include "RiverHead.h"
#include "Math/Header/BaseStruct.h"

struct Keyframe
{
	//Keyframe() {};
	//~Keyframe() {};

	float TimePos;
	Float3 Translation;
	Float3 Scale;
	Float4 RotationQuat;
};

struct BoneAnimation
{
	float GetStartTime() const;

	float GetEndTime() const;

	void Interpolate(float t, Matrix4x4& M) const;

	V_Array<Keyframe> Keyframes;
};

struct AnimationClip
{
	float GetClipStartTime() const;

	float GetClipEndTime() const;

	void Interpolate(float t, V_Array<Matrix4x4>& boneTransforms)const;

	V_Array<BoneAnimation> BoneAnimations;
};