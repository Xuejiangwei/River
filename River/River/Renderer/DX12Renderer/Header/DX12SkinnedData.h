#ifndef SKINNEDDATA_H
#define SKINNEDDATA_H

#include <vector>
#include <unordered_map>
#include <string>
#include <vector>
#include "RiverHead.h"
#include "DX12Util.h"

//class SkinnedData
//{
//public:
//
//	UINT BoneCount()const;
//
//	float GetClipStartTime(const std::string& clipName)const;
//	float GetClipEndTime(const std::string& clipName)const;
//
//	void Set(
//		std::vector<int>& boneHierarchy,
//		std::vector<DirectX::XMFLOAT4X4>& boneOffsets,
//		std::unordered_map<std::string, AnimationClip>& animations);
//
//	// In a real project, you'd want to cache the result if there was a chance
//	// that you were calling this several times with the same clipName at 
//	// the same timePos.
//	void GetFinalTransforms(const std::string& clipName, float timePos,
//		std::vector<DirectX::XMFLOAT4X4>& finalTransforms)const;
//
//private:
//	// Gives parentIndex of ith bone.
//	std::vector<int> mBoneHierarchy;
//
//	std::vector<DirectX::XMFLOAT4X4> mBoneOffsets;
//
//	std::unordered_map<std::string, AnimationClip> mAnimations;
//};

#endif // SKINNEDDATA_H