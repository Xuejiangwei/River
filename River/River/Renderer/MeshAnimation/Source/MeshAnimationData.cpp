#include "RiverPch.h"
#include "Renderer/MeshAnimation/Header/MeshAnimationData.h"
#include "Math/Header/BaseFunction.h"
#include "Math/Header/Geometric.h"

float AnimationClip::GetClipStartTime() const
{
	float t = FLT_MAX;
	for (uint32 i = 0; i < BoneAnimations.size(); ++i)
	{
		t = Min(t, BoneAnimations[i].GetStartTime());
	}

	return t;
}

float AnimationClip::GetClipEndTime() const
{
	float t = 0.0f;
	for (uint32 i = 0; i < BoneAnimations.size(); ++i)
	{
		t = Max(t, BoneAnimations[i].GetEndTime());
	}

	return t;
}

void AnimationClip::Interpolate(float t, V_Array<Matrix4x4>& boneTransforms) const
{
	for (uint32 i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

float BoneAnimation::GetStartTime() const
{
	return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime() const
{
	return Keyframes.back().TimePos;
}

void BoneAnimation::Interpolate(float t, Matrix4x4& M) const
{
	if (t <= Keyframes.front().TimePos)
	{
		auto S = Keyframes.front().Scale;
		auto P = Keyframes.front().Translation;
		auto Q = Keyframes.front().RotationQuat;

		Float4 zero(0.0f, 0.0f, 0.0f, 1.0f);
		M = Matrix4x4_AffineTransformation(S, zero, Q, P);
	}
	else if (t >= Keyframes.back().TimePos)
	{
		auto S = Keyframes.back().Scale;
		auto P = Keyframes.back().Translation;
		auto Q = Keyframes.back().RotationQuat;

		Float4 zero(0.0f, 0.0f, 0.0f, 1.0f);
		M = Matrix4x4_AffineTransformation(S, zero, Q, P);
	}
	else
	{
		for (auto i = 0; i < Keyframes.size() - 1; ++i)
		{
			if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
			{
				float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

				auto s0 = Keyframes[i].Scale;
				auto s1 = Keyframes[i + 1].Scale;

				auto p0 = Keyframes[i].Translation;
				auto p1 = Keyframes[i + 1].Translation;

				auto q0 = Keyframes[i].RotationQuat;
				auto q1 = Keyframes[i + 1].RotationQuat;

				auto S = VectorLerp(s0, s1, lerpPercent);
				auto P = VectorLerp(p0, p1, lerpPercent);
				auto Q = QuaternionSlerp(q0, q1, lerpPercent);

				auto zero = Float4(0.0f, 0.0f, 0.0f, 1.0f);
				M = Matrix4x4_AffineTransformation(S, zero, Q, P);

				break;
			}
		}
	}
}
