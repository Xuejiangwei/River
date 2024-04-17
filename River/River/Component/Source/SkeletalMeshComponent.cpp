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
	//m_AnimTimePos = 0.837342f;//+= deltaTime;
	m_AnimTimePos += deltaTime;

	auto& skelatalData = m_SkeletalMesh->GetSkeletalMeshData();
	auto& currAnim = skelatalData->AnimClips["Take1"];
	if (m_AnimTimePos > currAnim.GetClipEndTime())
	{
		m_AnimTimePos = 0.f;
	}

	auto numBones = skelatalData->BoneOffsets.size();

	m_FinalTransforms.resize(skelatalData->BoneHierarchy.size());
	std::vector<Matrix4x4> toParentTransforms(numBones);

	currAnim.Interpolate(m_AnimTimePos, toParentTransforms);
	std::vector<Matrix4x4> toRootTransforms(numBones);

	toRootTransforms[0] = toParentTransforms[0];

	for (auto i = 1; i < numBones; ++i)
	{
		auto toParent = toParentTransforms[i];

		int parentIndex = skelatalData->BoneHierarchy[i];
		auto parentToRoot = toRootTransforms[parentIndex];

		auto toRoot = Matrix4x4::Multiply(toParent, parentToRoot);

		toRootTransforms[i] = toRoot;
	}

	// Premultiply by the bone offset transform to get the final transform.
	for (auto i = 0; i < numBones; ++i)
	{
		auto offset = skelatalData->BoneOffsets[i];
		auto toRoot = toRootTransforms[i];
		auto finalTransform = Matrix4x4::Multiply(offset, toRoot);
		m_FinalTransforms[i] = Matrix4x4_Transpose(finalTransform);
	}

	/*HAZE_LOG_ERR("%f\n", m_AnimTimePos);
	for (size_t i = 0; i < numBones; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				HAZE_LOG_INFO("%f ", m_FinalTransforms[i].m[j][k]);
			}
			HAZE_LOG_INFO("\n");
		}
		HAZE_LOG_INFO("\n");
	}

	HAZE_LOG_ERR("%f-----------------------------------------\n", m_AnimTimePos);*/
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
