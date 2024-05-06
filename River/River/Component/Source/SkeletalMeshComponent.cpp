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
	//auto& currAnim = skelatalData->AnimClips.find("Take1");
	auto& currAnim = skelatalData->AnimClips.find("anim0");
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

	currAnim->second.Interpolate(m_AnimTimePos, m_FinalTransforms);
	//currAnim->second.Interpolate(m_AnimTimePos, toParentTransforms);
	//std::vector<Matrix4x4> toRootTransforms(numBones);

	//for (auto i = 0; i < numBones; ++i)
	//{
	//	int parentIndex = skelatalData->BoneHierarchy[i];
	//	if (parentIndex < 0)
	//	{
	//		toRootTransforms[i] = toParentTransforms[0];
	//		continue;
	//	}

	//	auto toParent = toParentTransforms[i];
	//	auto parentToRoot = toRootTransforms[parentIndex];

	//	auto toRoot = Matrix4x4::Multiply(toParent, parentToRoot);

	//	toRootTransforms[i] = toRoot;
	//}

	//// Premultiply by the bone offset transform to get the final transform.
	//for (auto i = 0; i < numBones; ++i)
	//{
	//	auto offset = skelatalData->BoneOffsets[i];
	//	auto toRoot = toRootTransforms[i];
	//	auto finalTransform = Matrix4x4::Multiply(offset, toRoot);
	//	m_FinalTransforms[i] = Matrix4x4_Transpose(finalTransform);
	//}

	//float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	//weights[0] = 0.5;
	//weights[1] = 0.5;
	//weights[2] = 0;
	//weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	//Float3 posL = Float3(0.0f, 0.0f, 0.0f);
	//Float3 normalL = Float3(0.0f, 0.0f, 0.0f);
	//Float3 tangentL = Float3(0.0f, 0.0f, 0.0f);
	//float bi[4] = { 1,2,0,0 };

	//Float4 t;
	//Matrix4x4 m4;
	//memset(&t, 0, sizeof(t));
	//for (int i = 0; i < 4; ++i)
	//{
	//	// Assume no nonuniform scaling when transforming normals, so 
	//	// that we do not have to use the inverse-transpose.

	//	m4 = m_FinalTransforms[bi[i]] * weights[i] + m4;
	//	//t +=  Vector_Matrix4x4_Multiply(Float4(0.0f, -0.0581496917, 1.60415041, 0.0f), m_FinalTransforms[bi[i]]) * weights[i];
	//}

	//Float4 zero(0.0f, 0.0f, 0.0f, 1.0f);
	//
	//auto t1 = Matrix4x4::Multiply( Matrix4x4(
	//1,0,0,0,
	//0,1,0, 0,
	//0,0,1, 0,
	//0, -0.0581496917, 1.60415041,1
	//), m4);

	//t = Matrix4x4_Multiply_Vector(m4, Float4(0.0f, -0.0581496917, 1.60415041, 0.0f));
	//shader -0.145677164  -0.00986723229  1.59383941
	// t   0.127824336  -0.125367537  1.58965254
	//right value 0.067408972475347939  0.070884505274615928  1.6217098700999535
	
	//0.972700298  -0.190457702 -0.0977166668 0.00000000
	//0.190817639  0.976473570  0.0292456374  0.00000000
	//0.0866005570  -0.0427552797  0.992022395 0.00000000
	//-0.0604153648  0.196252048  0.0320572853  1.00000000
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
