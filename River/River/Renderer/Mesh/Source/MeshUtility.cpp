#include "RiverPch.h"
#include "Renderer/Mesh/Header/MeshUtility.h"
#include "Renderer/MeshAnimation/Header/MeshAnimationData.h"
#include "Renderer/Header/Material.h"
#include "Renderer/Texture/Header/Texture.h"
#include "Renderer/DX12Renderer/Header/DX12DefaultConfig.h"
#include "Renderer/Header/AssetManager.h"

#include <fstream>

extern bool LoadFbxMesh(const String& path, SkeletalMeshData* skeletalMeshData);

void ReadMaterials(std::ifstream& fin, uint32 numMaterials, V_Array<Material*>& mats);
void ReadSubsetTable(std::ifstream& fin, uint32 numSubsets, std::vector<SkeletalSubset>& subsets);
void ReadVertices(std::ifstream& fin, uint32 numVertices, std::vector<Vertex>& vertices);
void ReadSkinnedVertices(std::ifstream& fin, uint32 numVertices, std::vector<SkeletalVertex>& vertices);
void ReadTriangles(std::ifstream& fin, uint32 numTriangles, std::vector<uint32>& indices);
void ReadBoneOffsets(std::ifstream& fin, uint32 numBones, std::vector<Matrix4x4>& boneOffsets);
void ReadBoneHierarchy(std::ifstream& fin, uint32 numBones, std::vector<int>& boneIndexToParentIndex);
void ReadAnimationClips(std::ifstream& fin, uint32 numBones, uint32 numAnimationClips, 
	HashMap<String, AnimationClip>& animations);
void ReadBoneKeyframes(std::ifstream& fin, uint32 numBones, BoneAnimation& boneAnimation);

void WriteToFile(SkeletalMeshData* skeletalMeshData);

bool LoadStaticMesh(const String& path, StaticMeshData* meshData)
{
	return false;
}

bool LoadSkeletalMesh(const String& path, SkeletalMeshData* skeletalMeshData)
{
	auto i = path.find(".fbx", path.length() - 4);
	if (i == String::npos)
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

			ReadMaterials(fin, numMaterials, skeletalMeshData->Materials);
			ReadSubsetTable(fin, numMaterials, skeletalMeshData->Subsets);
			ReadSkinnedVertices(fin, numVertices, skeletalMeshData->Vertices);
			ReadTriangles(fin, numTriangles, skeletalMeshData->Indices);
			ReadBoneOffsets(fin, numBones, skeletalMeshData->BoneOffsets);
			ReadBoneHierarchy(fin, numBones, skeletalMeshData->BoneHierarchy);
			ReadAnimationClips(fin, numBones, numAnimationClips, skeletalMeshData->AnimClips);

			return true;
		}
	}
	else
	{
		return LoadFbxMesh(path, skeletalMeshData);
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

		if (ignore != "Skinned")
		{
			mats[i]->m_Shader = AssetManager::Get()->GetShader("opaque");
		}

		String textureName;
		fin >> ignore >> textureName;
		auto texture = Texture::CreateTexture(textureName.c_str(), (DEFAULT_TEXTURE_PATH + textureName).c_str());
		mats[i]->m_DiffuseTexture = texture;

		textureName.clear();
		fin >> ignore >> textureName;
		texture = Texture::CreateTexture(textureName.c_str(), (DEFAULT_TEXTURE_PATH + textureName).c_str());
		mats[i]->m_NormalTexture = texture;

		mats[i]->MatCBIndex = 1;
	}
}

void ReadSubsetTable(std::ifstream& fin, uint32 numSubsets, std::vector<SkeletalSubset>& subsets)
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
		subsets[i].IndexStart *= 3;
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

void WriteToFile(SkeletalMeshData* skeletalMeshData)
{
	// Ð´ÈëÎÄ¼þ
	std::ofstream meshFile("F:\\GitHub\\River\\River\\Models\\women.m3d");

	meshFile << "***************m3d-File-Header*******************" << std::endl;
	meshFile << "#Materials " << skeletalMeshData->Materials.size() << std::endl;
	meshFile << "#Vertices " << skeletalMeshData->Vertices.size() << std::endl;
	meshFile << "#Triangles " << skeletalMeshData->Indices.size() / 3 << std::endl;
	meshFile << "#Bones " << skeletalMeshData->BoneHierarchy.size() << std::endl;
	meshFile << "#AnimationClips " << skeletalMeshData->AnimClips.size() << std::endl << std::endl;

	meshFile << "***************Materials*********************" << std::endl;
	for (size_t i = 0; i < skeletalMeshData->Materials.size(); i++)
	{
		meshFile << "Name: " << skeletalMeshData->Materials[i]->m_Name + "1" << std::endl;
		meshFile << "Diffuse: " << skeletalMeshData->Materials[i]->DiffuseAlbedo.r << " " << skeletalMeshData->Materials[i]->DiffuseAlbedo.g << " "
			<< skeletalMeshData->Materials[i]->DiffuseAlbedo.b << std::endl;
		meshFile << "Fresnel0: " << skeletalMeshData->Materials[i]->FresnelR0.x << " " << skeletalMeshData->Materials[i]->FresnelR0.y << " "
			<< skeletalMeshData->Materials[i]->FresnelR0.z << std::endl;
		meshFile << "Roughness: " << skeletalMeshData->Materials[i]->Roughness << std::endl;
		meshFile << "AlphaClip: " << 0 << std::endl;
		meshFile << "MaterialTypeName: " << "Skinned" << std::endl;
		meshFile << "DiffuseMap: " << "fbx_extra_jiulian.dds" << std::endl;
		meshFile << "NormalMap: " << "jacket_norm.dds" << std::endl;
	}
	meshFile << std::endl;

	meshFile << "***************SubsetTable*******************" << std::endl;
	for (size_t i = 0; i < skeletalMeshData->Subsets.size(); i++)
	{
		meshFile << "SubsetID: " << skeletalMeshData->Subsets[i].Id << " VertexStart: " << skeletalMeshData->Subsets[i].VertexStart << " VertexCount: " << skeletalMeshData->Subsets[i].VertexCount
			<< " FaceStart: " << skeletalMeshData->Subsets[i].IndexStart / 3 << " FaceCount: " << skeletalMeshData->Subsets[i].IndexCount / 3 << std::endl;
	}
	meshFile << std::endl;

	meshFile << "***************Vertices**********************" << std::endl;
	for (size_t i = 0; i < skeletalMeshData->Vertices.size(); i++)
	{
		meshFile << "Position: " << skeletalMeshData->Vertices[i].Pos.x << " " << skeletalMeshData->Vertices[i].Pos.y << " " <<
			skeletalMeshData->Vertices[i].Pos.z << std::endl;
		meshFile << "Tangent: " << skeletalMeshData->Vertices[i].TangentU.x << " " << skeletalMeshData->Vertices[i].TangentU.y << " " <<
			skeletalMeshData->Vertices[i].TangentU.z << " " << 1 << std::endl;
		meshFile << "Normal: " << skeletalMeshData->Vertices[i].Normal.x << " " << skeletalMeshData->Vertices[i].Normal.y << " " <<
			skeletalMeshData->Vertices[i].Normal.z << std::endl;
		meshFile << "Tex-Coords: " << skeletalMeshData->Vertices[i].TexC.x << " " << skeletalMeshData->Vertices[i].TexC.y << std::endl;
		meshFile << "BlendWeights: " << skeletalMeshData->Vertices[i].BoneWeights.x << " " << skeletalMeshData->Vertices[i].BoneWeights.y << " " <<
			skeletalMeshData->Vertices[i].BoneWeights.z << " " <<
			(1 - (skeletalMeshData->Vertices[i].BoneWeights.x + skeletalMeshData->Vertices[i].BoneWeights.y + skeletalMeshData->Vertices[i].BoneWeights.z)) << std::endl;
		meshFile << "BlendIndices: " << (int)skeletalMeshData->Vertices[i].BoneIndices[0] << " " << (int)skeletalMeshData->Vertices[i].BoneIndices[1] << " " <<
			(int)skeletalMeshData->Vertices[i].BoneIndices[2] << " " << (int)skeletalMeshData->Vertices[i].BoneIndices[3] << std::endl;

		meshFile << std::endl;
	}

	meshFile << "***************Triangles*********************" << std::endl;
	for (size_t i = 0; i < skeletalMeshData->Indices.size(); i++)
	{
		meshFile << skeletalMeshData->Indices[i];
		if (i > 0 && (i + 1) % 3 == 0)
		{
			meshFile << std::endl;
		}
		else
		{
			meshFile << " ";
		}
	}
	meshFile << std::endl;

	meshFile << "***************BoneOffsets*******************" << std::endl;
	for (size_t i = 0; i < skeletalMeshData->BoneOffsets.size(); i++)
	{
		meshFile << "BoneOffset" << i << " " <<
			skeletalMeshData->BoneOffsets[i](0, 0) << " " << skeletalMeshData->BoneOffsets[i](0, 1) << " " << skeletalMeshData->BoneOffsets[i](0, 2) << " " << skeletalMeshData->BoneOffsets[i](0, 3) << " " <<
			skeletalMeshData->BoneOffsets[i](1, 0) << " " << skeletalMeshData->BoneOffsets[i](1, 1) << " " << skeletalMeshData->BoneOffsets[i](1, 2) << " " << skeletalMeshData->BoneOffsets[i](1, 3) << " " <<
			skeletalMeshData->BoneOffsets[i](2, 0) << " " << skeletalMeshData->BoneOffsets[i](2, 1) << " " << skeletalMeshData->BoneOffsets[i](2, 2) << " " << skeletalMeshData->BoneOffsets[i](2, 3) << " " <<
			skeletalMeshData->BoneOffsets[i](3, 0) << " " << skeletalMeshData->BoneOffsets[i](3, 1) << " " << skeletalMeshData->BoneOffsets[i](3, 2) << " " << skeletalMeshData->BoneOffsets[i](3, 3) <<
			std::endl;
	}
	meshFile << std::endl;

	meshFile << "***************BoneHierarchy*****************" << std::endl;
	for (size_t i = 0; i < skeletalMeshData->BoneHierarchy.size(); i++)
	{
		meshFile << "ParentIndexOfBone" << i << ": " << skeletalMeshData->BoneHierarchy[i] << std::endl;
	}
	meshFile << std::endl;

	meshFile << "***************AnimationClips****************" << std::endl;
	for (auto& clip : skeletalMeshData->AnimClips)
	{
		meshFile << "AnimationClip " << clip.first << std::endl;
		meshFile << "{" << std::endl;
		for (size_t i = 0; i < clip.second.BoneAnimations.size(); i++)
		{
			meshFile << "	Bone" << i << " #Keyframes: " << clip.second.BoneAnimations[i].Keyframes.size() << std::endl;
			meshFile << "	{" << std::endl;

			for (auto& frame : clip.second.BoneAnimations[i].Keyframes)
			{
				meshFile << "		Time: " << frame.TimePos << " Pos: " << frame.Translation.x << " " << frame.Translation.y << " " <<
					frame.Translation.z << " Scale: " << frame.Scale.x << " " << frame.Scale.y << " " << frame.Scale.z <<
					" Quat: " << frame.RotationQuat.x << " " << frame.RotationQuat.y << " " << frame.RotationQuat.z << " " <<
					frame.RotationQuat.w << std::endl;
			}

			meshFile << "	}" << std::endl << std::endl;
		}
		meshFile << "}" << std::endl;
	}
}