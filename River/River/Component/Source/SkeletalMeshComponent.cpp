#include "RiverPch.h"
#include "Component/Header/SkeletalMeshComponent.h"
#include "Renderer/Mesh/Header/SkeletalMesh.h"
#include "Math/Header/Geometric.h"

#include "Haze/include/HazeLog.h"

SkeletalMeshComponent::SkeletalMeshComponent()
	: m_SkeletalMesh(nullptr), m_AnimTimePos(0.f)
{
}

SkeletalMeshComponent::~SkeletalMeshComponent()
{
}

void SkeletalMeshComponent::Tick(float deltaTime)
{
	static bool bset = false;

	//m_AnimTimePos = 0.837342f;//+= deltaTime;
	m_AnimTimePos += deltaTime;

	auto& skelatalData = m_SkeletalMesh->GetSkeletalMeshData();
	//auto& currAnim = skelatalData->AnimClips.find("Take1");
	auto& currAnim = skelatalData->AnimClips.find("anim1");
	if (currAnim == skelatalData->AnimClips.end())
	{
		return;
	}

	if (m_AnimTimePos > currAnim->second.GetClipEndTime())
	{
		m_AnimTimePos = 0.f;
	}

	//m_AnimTimePos = 0.0f;

	auto numBones = skelatalData->BoneOffsets.size();

	m_FinalTransforms.resize(skelatalData->BoneHierarchy.size());
	std::vector<Matrix4x4> toParentTransforms(numBones);

	//currAnim->second.Interpolate(m_AnimTimePos, m_FinalTransforms);
	currAnim->second.Interpolate(m_AnimTimePos, toParentTransforms);
	std::vector<Matrix4x4> toRootTransforms(numBones);

	for (auto i = 0; i < numBones; ++i)
	{
		bool log = i == 0;
		int parentIndex = skelatalData->BoneHierarchy[i];
		if (parentIndex < 0)
		{
			toRootTransforms[i] = toParentTransforms[i];

			if (log && !bset)
			{
				LOG("Toroot %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f",
					toRootTransforms[i].m[0][0], toRootTransforms[i].m[0][1], toRootTransforms[i].m[0][2], toRootTransforms[i].m[0][3],
					toRootTransforms[i].m[1][0], toRootTransforms[i].m[1][1], toRootTransforms[i].m[1][2], toRootTransforms[i].m[1][3],
					toRootTransforms[i].m[2][0], toRootTransforms[i].m[2][1], toRootTransforms[i].m[2][2], toRootTransforms[i].m[2][3],
					toRootTransforms[i].m[3][0], toRootTransforms[i].m[3][1], toRootTransforms[i].m[3][2], toRootTransforms[i].m[3][3]);
			}

			continue;
		}

		auto toParent = toParentTransforms[i];
		auto parentToRoot = toRootTransforms[parentIndex];
		auto toRoot = Matrix4x4::Multiply(toParent, parentToRoot);

		if (log && !bset)
		{
			LOG("toParent %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f",
				toParent.m[0][0], toParent.m[0][1], toParent.m[0][2], toParent.m[0][3],
				toParent.m[1][0], toParent.m[1][1], toParent.m[1][2], toParent.m[1][3],
				toParent.m[2][0], toParent.m[2][1], toParent.m[2][2], toParent.m[2][3],
				toParent.m[3][0], toParent.m[3][1], toParent.m[3][2], toParent.m[3][3]);

			LOG("parentToRoot %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f",
				parentToRoot.m[0][0], parentToRoot.m[0][1], parentToRoot.m[0][2], parentToRoot.m[0][3],
				parentToRoot.m[1][0], parentToRoot.m[1][1], parentToRoot.m[1][2], parentToRoot.m[1][3],
				parentToRoot.m[2][0], parentToRoot.m[2][1], parentToRoot.m[2][2], parentToRoot.m[2][3],
				parentToRoot.m[3][0], parentToRoot.m[3][1], parentToRoot.m[3][2], parentToRoot.m[3][3]);

			LOG("toRoot %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f",
				toRoot.m[0][0], toRoot.m[0][1], toRoot.m[0][2], toRoot.m[0][3],
				toRoot.m[1][0], toRoot.m[1][1], toRoot.m[1][2], toRoot.m[1][3],
				toRoot.m[2][0], toRoot.m[2][1], toRoot.m[2][2], toRoot.m[2][3],
				toRoot.m[3][0], toRoot.m[3][1], toRoot.m[3][2], toRoot.m[3][3]);
		}

		toRootTransforms[i] = toRoot;
	}

	// Premultiply by the bone offset transform to get the final transform.

	
	for (auto i = 0; i < numBones; ++i)
	{
		bool log = i == 0;
		auto offset = skelatalData->BoneOffsets[i];
		auto toRoot = toRootTransforms[i];
		auto finalTransform = Matrix4x4::Multiply(offset, toRoot);
		//finalTransform = Matrix4x4::Multiply(finalTransform, skelatalData->BoneOffsets1[i]);
		

		if (log && !bset)
		{
			LOG("offset %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f",
				offset.m[0][0], offset.m[0][1], offset.m[0][2], offset.m[0][3],
				offset.m[1][0], offset.m[1][1], offset.m[1][2], offset.m[1][3],
				offset.m[2][0], offset.m[2][1], offset.m[2][2], offset.m[2][3],
				offset.m[3][0], offset.m[3][1], offset.m[3][2], offset.m[3][3]);

			/*LOG("offset1 %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f",
				skelatalData->BoneOffsets1[i].m[0][0], skelatalData->BoneOffsets1[i].m[0][1], skelatalData->BoneOffsets1[i].m[0][2], skelatalData->BoneOffsets1[i].m[0][3],
				skelatalData->BoneOffsets1[i].m[1][0], skelatalData->BoneOffsets1[i].m[1][1], skelatalData->BoneOffsets1[i].m[1][2], skelatalData->BoneOffsets1[i].m[1][3],
				skelatalData->BoneOffsets1[i].m[2][0], skelatalData->BoneOffsets1[i].m[2][1], skelatalData->BoneOffsets1[i].m[2][2], skelatalData->BoneOffsets1[i].m[2][3],
				skelatalData->BoneOffsets1[i].m[3][0], skelatalData->BoneOffsets1[i].m[3][1], skelatalData->BoneOffsets1[i].m[3][2], skelatalData->BoneOffsets1[i].m[3][3]);*/

			LOG("curr %d %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f\n", i,
				finalTransform.m[0][0], finalTransform.m[0][1], finalTransform.m[0][2], finalTransform.m[0][3],
				finalTransform.m[1][0], finalTransform.m[1][1], finalTransform.m[1][2], finalTransform.m[1][3],
				finalTransform.m[2][0], finalTransform.m[2][1], finalTransform.m[2][2], finalTransform.m[2][3],
				finalTransform.m[3][0], finalTransform.m[3][1], finalTransform.m[3][2], finalTransform.m[3][3]);
		}

		m_FinalTransforms[i] = Matrix4x4_Transpose(finalTransform);

	}
	bset = true;
}

void SkeletalMeshComponent::SetSkeletalMesh(SkeletalMesh* skeletalMesh)
{
	m_SkeletalMesh = skeletalMesh;
}

const V_Array<class Material*>& SkeletalMeshComponent::GetSkeletalMeshMaterials() const
{
	return m_SkeletalMesh->GetSkeletalMeshData()->Materials;
}

MaterialBlendMode SkeletalMeshComponent::GetMaterialMode() const
{
	return m_Materials.size() > 0 ? m_Materials[0]->m_BlendMode : MaterialBlendMode::Opaque;
}
