#include "RiverPch.h"
#include "Renderer/Mesh/Header/MeshUtility.h"
#include "Renderer/MeshAnimation/Header/MeshAnimationData.h"
#include "Renderer/Header/Material.h"
#include "Renderer/Header/Texture.h"
#include "Renderer/DX12Renderer/Header/DX12DefaultConfig.h"
#include "Renderer/Header/AssetManager.h"

#include <fstream>

void ReadMaterials(std::ifstream& fin, uint32 numMaterials, V_Array<Material*>& mats);
void ReadSubsetTable(std::ifstream& fin, uint32 numSubsets, std::vector<Subset>& subsets);
void ReadVertices(std::ifstream& fin, uint32 numVertices, std::vector<Vertex>& vertices);
void ReadSkinnedVertices(std::ifstream& fin, uint32 numVertices, std::vector<SkeletalVertex>& vertices);
void ReadTriangles(std::ifstream& fin, uint32 numTriangles, std::vector<uint32>& indices);
void ReadBoneOffsets(std::ifstream& fin, uint32 numBones, std::vector<Matrix4x4>& boneOffsets);
void ReadBoneHierarchy(std::ifstream& fin, uint32 numBones, std::vector<int>& boneIndexToParentIndex);
void ReadAnimationClips(std::ifstream& fin, uint32 numBones, uint32 numAnimationClips, 
	HashMap<String, AnimationClip>& animations);
void ReadBoneKeyframes(std::ifstream& fin, uint32 numBones, BoneAnimation& boneAnimation);

bool LoadStaticMesh(const String& path, StaticMeshData* meshData)
{
	return false;
}

bool LoadSkeletalMesh(const String& path, SkeletalMeshData* skeletalMeshData)
{
	std::ifstream fin(path);

	uint32 numMaterials = 0;
	uint32 numVertices = 0;
	uint32 numTriangles = 0;
	uint32 numBones = 0;
	uint32 numAnimationClips = 0;

	std::string ignore;

	if (fin)
	{
		fin >> ignore; // file header text
		fin >> ignore >> numMaterials;
		fin >> ignore >> numVertices;
		fin >> ignore >> numTriangles;
		fin >> ignore >> numBones;
		fin >> ignore >> numAnimationClips;

		std::vector<Subset> subsets;
		ReadMaterials(fin, numMaterials, skeletalMeshData->Materials);
		ReadSubsetTable(fin, numMaterials, subsets);
		ReadSkinnedVertices(fin, numVertices, skeletalMeshData->Vertices);
		ReadTriangles(fin, numTriangles, skeletalMeshData->Indices);
		ReadBoneOffsets(fin, numBones, skeletalMeshData->BoneOffsets);
		ReadBoneHierarchy(fin, numBones, skeletalMeshData->BoneHierarchy);
		ReadAnimationClips(fin, numBones, numAnimationClips, skeletalMeshData->AnimClips);

		return true;
	}
	return false;
}

void ReadMaterials(std::ifstream& fin, uint32 numMaterials, V_Array<Material*>& mats)
{
	std::string ignore;
	mats.resize(numMaterials);

	std::string diffuseMapName;
	std::string normalMapName;

	fin >> ignore; // materials header text

	auto shader = AssetManager::Get()->GetShader("skeletalOpaque");
	for (uint32 i = 0; i < numMaterials; ++i)
	{
		fin >> ignore >> ignore;
		mats[i] = Material::CreateMaterial(ignore.c_str(), "");
		mats[i]->m_Shader = shader;
		fin >> ignore >> mats[i]->DiffuseAlbedo.x >> mats[i]->DiffuseAlbedo.y >> mats[i]->DiffuseAlbedo.z;
		fin >> ignore >> mats[i]->FresnelR0.x >> mats[i]->FresnelR0.y >> mats[i]->FresnelR0.z;
		fin >> ignore >> mats[i]->Roughness;
		fin >> ignore >> ignore;//mats[i]->AlphaClip;
		fin >> ignore >> ignore;//mats[i]->MaterialTypeName;

		String textureName;
		fin >> ignore >> textureName;
		auto texture = Texture::CreateTexture(textureName.c_str(), (DEFAULT_TEXTURE_PATH + textureName).c_str());
		mats[i]->m_DiffuseTexture = texture;

		textureName.clear();
		fin >> ignore >> textureName;
		texture = Texture::CreateTexture(textureName.c_str(), (DEFAULT_TEXTURE_PATH + textureName).c_str());
		mats[i]->m_NormalTexture = texture;
	}
}

void ReadSubsetTable(std::ifstream& fin, uint32 numSubsets, std::vector<Subset>& subsets)
{
	std::string ignore;
	subsets.resize(numSubsets);

	fin >> ignore; // subset header text
	for (uint32 i = 0; i < numSubsets; ++i)
	{
		fin >> ignore >> subsets[i].Id;
		fin >> ignore >> subsets[i].VertexStart;
		fin >> ignore >> subsets[i].VertexCount;
		fin >> ignore >> subsets[i].IndexStart;
		fin >> ignore >> subsets[i].IndexCount;

		subsets[i].IndexCount *= 3;
	}
}

void ReadVertices(std::ifstream& fin, uint32 numVertices, std::vector<Vertex>& vertices)
{
	float blah;

	std::string ignore;
	vertices.resize(numVertices);

	fin >> ignore; // vertices header text
	for (uint32 i = 0; i < numVertices; ++i)
	{
		fin >> ignore >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> ignore >> vertices[i].TangentU.x >> vertices[i].TangentU.y >> vertices[i].TangentU.z >> blah;
		fin >> ignore >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
		fin >> ignore >> vertices[i].TexC.x >> vertices[i].TexC.y;
	}
}

void ReadSkinnedVertices(std::ifstream& fin, uint32 numVertices, std::vector<SkeletalVertex>& vertices)
{
	std::string ignore;
	vertices.resize(numVertices);

	fin >> ignore; // vertices header text
	int boneIndices[4];
	float weights[4];
	for (uint32 i = 0; i < numVertices; ++i)
	{
		float blah;
		fin >> ignore >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> ignore >> vertices[i].TangentU.x >> vertices[i].TangentU.y >> vertices[i].TangentU.z >> blah /*vertices[i].TangentU.w*/;
		fin >> ignore >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
		fin >> ignore >> vertices[i].TexC.x >> vertices[i].TexC.y;
		fin >> ignore >> weights[0] >> weights[1] >> weights[2] >> weights[3];
		fin >> ignore >> boneIndices[0] >> boneIndices[1] >> boneIndices[2] >> boneIndices[3];

		vertices[i].BoneWeights.x = weights[0];
		vertices[i].BoneWeights.y = weights[1];
		vertices[i].BoneWeights.z = weights[2];

		vertices[i].BoneIndices[0] = (uint8)boneIndices[0];
		vertices[i].BoneIndices[1] = (uint8)boneIndices[1];
		vertices[i].BoneIndices[2] = (uint8)boneIndices[2];
		vertices[i].BoneIndices[3] = (uint8)boneIndices[3];
	}
}

void ReadTriangles(std::ifstream& fin, uint32 numTriangles, std::vector<uint32>& indices)
{
	std::string ignore;
	indices.resize(numTriangles * 3);

	fin >> ignore; // triangles header text
	for (uint32 i = 0; i < numTriangles; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}
}

void ReadBoneOffsets(std::ifstream& fin, uint32 numBones, std::vector<Matrix4x4>& boneOffsets)
{
	std::string ignore;
	boneOffsets.resize(numBones);

	fin >> ignore; // BoneOffsets header text
	for (uint32 i = 0; i < numBones; ++i)
	{
		fin >> ignore >>
			boneOffsets[i](0, 0) >> boneOffsets[i](0, 1) >> boneOffsets[i](0, 2) >> boneOffsets[i](0, 3) >>
			boneOffsets[i](1, 0) >> boneOffsets[i](1, 1) >> boneOffsets[i](1, 2) >> boneOffsets[i](1, 3) >>
			boneOffsets[i](2, 0) >> boneOffsets[i](2, 1) >> boneOffsets[i](2, 2) >> boneOffsets[i](2, 3) >>
			boneOffsets[i](3, 0) >> boneOffsets[i](3, 1) >> boneOffsets[i](3, 2) >> boneOffsets[i](3, 3);
	}
}

void ReadBoneHierarchy(std::ifstream& fin, uint32 numBones, std::vector<int>& boneIndexToParentIndex)
{
	std::string ignore;
	boneIndexToParentIndex.resize(numBones);

	fin >> ignore; // BoneHierarchy header text
	for (uint32 i = 0; i < numBones; ++i)
	{
		fin >> ignore >> boneIndexToParentIndex[i];
	}
}

void ReadAnimationClips(std::ifstream& fin, uint32 numBones, uint32 numAnimationClips,
	HashMap<String, AnimationClip>& animations)
{
	std::string ignore;
	fin >> ignore; // AnimationClips header text
	for (uint32 clipIndex = 0; clipIndex < numAnimationClips; ++clipIndex)
	{
		std::string clipName;
		fin >> ignore >> clipName;
		fin >> ignore; // {

		AnimationClip clip;
		clip.BoneAnimations.resize(numBones);

		for (uint32 boneIndex = 0; boneIndex < numBones; ++boneIndex)
		{
			ReadBoneKeyframes(fin, numBones, clip.BoneAnimations[boneIndex]);
		}
		fin >> ignore; // }

		animations[clipName] = clip;
	}
}

void ReadBoneKeyframes(std::ifstream& fin, uint32 numBones, BoneAnimation& boneAnimation)
{
	std::string ignore;
	uint32 numKeyframes = 0;
	fin >> ignore >> ignore >> numKeyframes;
	fin >> ignore; // {

	boneAnimation.Keyframes.resize(numKeyframes);
	for (uint32 i = 0; i < numKeyframes; ++i)
	{
		float t = 0.0f;
		Float3 p(0.0f, 0.0f, 0.0f);
		Float3 s(1.0f, 1.0f, 1.0f);
		Float4 q(0.0f, 0.0f, 0.0f, 1.0f);
		fin >> ignore >> t;
		fin >> ignore >> p.x >> p.y >> p.z;
		fin >> ignore >> s.x >> s.y >> s.z;
		fin >> ignore >> q.x >> q.y >> q.z >> q.w;

		boneAnimation.Keyframes[i].TimePos = t;
		boneAnimation.Keyframes[i].Translation = p;
		boneAnimation.Keyframes[i].Scale = s;
		boneAnimation.Keyframes[i].RotationQuat = q;
	}

	fin >> ignore; // }
}