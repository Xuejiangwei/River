#include "RiverPch.h"
#include "Math/Header/BaseStruct.h"
#include "Renderer/Mesh/Header/Mesh.h"
#include "Renderer/Header/AssetManager.h"

#include "fbxsdk/core/fbxmanager.h"
#include "fbxsdk/core/math/fbxvector4.h"
#include "fbxsdk.h"
#include "fbxsdk/core/fbxsystemunit.h"

#include <fstream>

#include "Math/Header/Geometric.h"

//#define USE_ASSIMP

//MappingMode定义了当前类型的元素如何映射到mesh上。举例来说，对于KFbxLayerElementNormal，eBY_POLYGON_VERTEX表示如果一个顶点被n个多边形共享
// ，那么这个顶点就有n条法线与之相对应；eBY_CONTROL_POINT则表示每个顶点无论被几个多边形共享，都只有一条normal；
// eBY_POLYGON则表示构成多边形的n个顶点只对应着一条normal。某些MappingMode只对特定的KFbxLayerElement有效，请详细参考文档。
// 通常对于有hard edge的模型来说，MappingMode只能是eBY_POLYGON_VERTEX，而平滑模型则可以是eBY_CONTROL_POINT

#define LOG_FBX_MATRIX(T) LOG("Matrix name:" #T ": %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f,  %f,%f,%f,%f", \
							T.Get(0, 0), T.Get(0, 1), T.Get(0, 2), T.Get(0, 3), \
							T.Get(1, 0), T.Get(1, 2), T.Get(1, 3), T.Get(1, 0), \
							T.Get(2, 0), T.Get(2, 2), T.Get(2, 3), T.Get(2, 0), \
							T.Get(3, 0), T.Get(3, 2), T.Get(3, 3), T.Get(3, 0))

extern void WriteToFile(SkeletalMeshData* skeletalMeshData);

bool LoadFbxMesh(const String& path, SkeletalMeshData* skeletalMeshData);

void Fbx_ParseMesh(const FbxMesh* pMesh, SkeletalMeshData* skeletalMeshData);
void Fbx_ParseNode(FbxNode* pNode, SkeletalMeshData* skeletalMeshData, int parentSkeletalIndex = -1);
void Fbx_ParseAniamtion(FbxNode* pNode, SkeletalMeshData* skeletalMeshData);

static FbxManager* s_FbxSdkManager = nullptr;
static FbxScene* s_FbxScene = nullptr;
static int s_SkeletalIndex = -1;
static V_Array<Pair<String, Pair<FbxNode*, FbxCluster*>>> s_Bones;

struct VertexRelateBoneInfo
{
	char bone_index_[4];	//骨骼索引，一般骨骼少于128个，用char就行。
	char bone_weight_[4];	//骨骼权重，权重不会超过100，所以用char类型就可以。
	VertexRelateBoneInfo()
	{
		for (int i = 0; i < _countof(bone_index_); ++i)
		{
			bone_index_[i] = -1;
		}
		for (int i = 0; i < _countof(bone_weight_); ++i)
		{
			bone_weight_[i] = 0;
		}
	}
	void Push(char bone_index, char bone_weight)
	{
		for (int i = 0; i < _countof(bone_index_); ++i)
		{
			if (bone_index_[i] == -1)
			{
				bone_index_[i] = bone_index;
				if (bone_weight_[i] == 0)
				{
					bone_weight_[i] = bone_weight;
					return;
				}

				LOG("bone_index_ bone_weight_ not sync");
			}
		}

		LOG("too much bone");
	}
};

Matrix4x4 FbxMatrixToGlmMat4(FbxAMatrix& fbxAMatrix)
{
	Matrix4x4 matrix;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			matrix.m[i][j] = (float)fbxAMatrix[i][j];
		}
	}

	return matrix;
}

void ComputeClusterDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxCluster* pCluster, FbxAMatrix& pBoneTransformMatrix, FbxTime pTime)
{
	FbxCluster::ELinkMode lClusterMode = pCluster->GetLinkMode();

	if (lClusterMode == FbxCluster::eNormalize)
	{
		FbxAMatrix clusterGlobalInitPosition;
		FbxAMatrix lAssociateGlobalInitPosition;
		FbxAMatrix lAssociateGlobalCurrentPosition;
		FbxAMatrix clusterBoneGlobalInitPosition;
		FbxAMatrix clusterBoneGlobalCurrentPosition;

		FbxAMatrix lReferenceGeometry;
		FbxAMatrix lAssociateGeometry;
		FbxAMatrix lClusterGeometry;

		FbxAMatrix lClusterRelativeInitPosition;
		FbxAMatrix lClusterRelativeCurrentPositionInverse;


		pCluster->GetTransformMatrix(clusterGlobalInitPosition);
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation

		//获得Mesh节点相对坐标系源点的平移、旋转和缩放
		const FbxVector4 lT = pMesh->GetNode()->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 lR = pMesh->GetNode()->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 lS = pMesh->GetNode()->GetGeometricScaling(FbxNode::eSourcePivot);

		lReferenceGeometry = FbxAMatrix(lT, lR, lS);
		clusterGlobalInitPosition *= lReferenceGeometry;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(clusterBoneGlobalInitPosition);
		clusterBoneGlobalCurrentPosition = pCluster->GetLink()->EvaluateGlobalTransform(pTime);

		// Compute the initial position of the link relative to the reference.

		lClusterRelativeInitPosition = clusterBoneGlobalInitPosition.Inverse() * clusterGlobalInitPosition;

		// Compute the current position of the link relative to the reference.
		lClusterRelativeCurrentPositionInverse = pGlobalPosition.Inverse() * clusterBoneGlobalCurrentPosition;

		// Compute the shift of the link relative to the reference.
		pBoneTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
		pBoneTransformMatrix = pGlobalPosition.Inverse() * (clusterBoneGlobalCurrentPosition * clusterBoneGlobalInitPosition.Inverse() * clusterGlobalInitPosition);
	}
}

#ifdef USE_ASSIMP
std::vector<String> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene, SkeletalMeshData* skeletalMeshData)
{
	std::vector<String> textures;
	for (uint32 i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;

		auto shader = AssetManager::Get()->GetShader("skeletalOpaque");//
		auto mat = Material::CreateMaterial("womenMat");
		mat->MatCBIndex = 1;
		mat->m_Shader = shader;
		mat->m_DiffuseTexture = AssetManager::Get()->GetTexture("jiulian");
		mat->m_NormalTexture = AssetManager::Get()->GetTexture("bricksNormalMap");
		skeletalMeshData->Materials.push_back(mat);

		SkeletalSubset subset;
		subset.Id = 1;
		subset.IndexCount = skeletalMeshData->Indices.size();
		subset.VertexCount = skeletalMeshData->Vertices.size();
		skeletalMeshData->Subsets.push_back(subset);

		textures.push_back(str.C_Str());
	}
	return textures;
}


void ProcessMesh(aiMesh* mesh, const aiScene* scene, SkeletalMeshData* skeletalMeshData)
{
	// Walk through each of the mesh's vertices

	skeletalMeshData->Vertices.resize(mesh->mNumVertices);
	for (uint32 i = 0; i < mesh->mNumVertices; i++)
	{
		skeletalMeshData->Vertices[i].Pos.x = mesh->mVertices[i].x;
		skeletalMeshData->Vertices[i].Pos.y = mesh->mVertices[i].y;
		skeletalMeshData->Vertices[i].Pos.z = mesh->mVertices[i].z;

		if (mesh->mTextureCoords[0])
		{
			skeletalMeshData->Vertices[i].TexC.x = (float)mesh->mTextureCoords[0][i].x;
			skeletalMeshData->Vertices[i].TexC.y = (float)mesh->mTextureCoords[0][i].y;
		}

		memset(&skeletalMeshData->Vertices[i].BoneIndices, (uint8)-1, 4);
	}


	for (uint32 i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (uint32 j = 0; j < face.mNumIndices; j++)
		{
			skeletalMeshData->Indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<String> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene, skeletalMeshData);
	}

	//设置骨骼层级
	aiNode* root = nullptr;
	HashMap<String, int> indexs;

	int boneIndx = 0;
	if (mesh->mNumBones > 0)
	{
		auto node = scene->mRootNode->FindNode(mesh->mBones[0]->mName);
		if (node && node->mParent)
		{
			indexs[node->mParent->mName.C_Str()] = boneIndx++;
		}
		skeletalMeshData->BoneHierarchy.push_back(-1);

		Matrix4x4 mat;
		auto m = node->mParent->mTransformation;
		m.Inverse();
		memcpy(&mat, &m, sizeof(Matrix4x4));
		skeletalMeshData->BoneOffsets.push_back(mat);
	}

	assert(mesh->mNumBones < uint8(-1));
	for (size_t i = 0; i < mesh->mNumBones; i++)
	{
		auto bone = mesh->mBones[i];
		LOG("node name %s", bone->mName.C_Str());

		for (size_t j = 0; j < bone->mNumWeights; j++)
		{
			auto widget = bone->mWeights[j];
			auto& boneIndices = skeletalMeshData->Vertices[widget.mVertexId].BoneIndices;
			auto& boneWidgets = skeletalMeshData->Vertices[widget.mVertexId].BoneWeights;
			for (size_t k = 0; k < _countof(boneIndices); k++)
			{
				if (boneIndices[k] == (uint8)-1)
				{
					boneIndices[k] = i;
					if (k < 4)
					{
						boneWidgets[k] = widget.mWeight;
					}
					break;
				}
			}
		}

		auto node = scene->mRootNode->FindNode(bone->mName);
		if (node)
		{
			auto iter = indexs.find(node->mParent->mName.C_Str());
			if (iter != indexs.end())
			{
				skeletalMeshData->BoneHierarchy.push_back(iter->second);

				Matrix4x4 mat;
				memcpy(&mat, &bone->mOffsetMatrix, sizeof(Matrix4x4));
				skeletalMeshData->BoneOffsets.push_back(mat);
				indexs[bone->mName.C_Str()] = boneIndx++;
			}
			else
			{
				LOG("Error");
			}
		}
		else
		{
			LOG("node name");
		}
	}

	for (size_t i = 0; i < scene->mNumAnimations; i++)
	{
		auto anim = scene->mAnimations[i];
		AnimationClip animationClip;
		animationClip.BoneAnimations.resize(anim->mNumChannels);
		for (size_t j = 0; j < anim->mNumChannels; j++)
		{
			auto& boneAnimation = animationClip.BoneAnimations[j];
			auto channel = anim->mChannels[j];
			boneAnimation.Keyframes.resize(channel->mNumPositionKeys);
			for (size_t k = 0; k < channel->mNumPositionKeys; k++)
			{
				memcpy(&boneAnimation.Keyframes[k].Translation, &channel->mPositionKeys[k].mValue, sizeof(Float3));
				memcpy(&boneAnimation.Keyframes[k].RotationQuat, &channel->mRotationKeys[k].mValue, sizeof(Float4));
				memcpy(&boneAnimation.Keyframes[k].Scale, &channel->mScalingKeys[k].mValue, sizeof(Float3));
			}

			LOG("r  node name %s", channel->mNodeName.C_Str());
		}

		skeletalMeshData->AnimClips["anim" + std::to_string(i)] = animationClip;
	}
}

void ProcessNode(aiNode* node, const aiScene* scene, SkeletalMeshData* skeletalMeshData)
{
	for (uint32 i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, scene, skeletalMeshData);
	}

	for (uint32 i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, skeletalMeshData);
	}
}

#endif // USE_ASSIMP


bool LoadFbxMesh(const String& path, SkeletalMeshData* skeletalMeshData)
{
#ifdef USE_ASSIMP
	Assimp::Importer importer;
	auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
	ProcessNode(scene->mRootNode, scene, skeletalMeshData);

	return true;
#else

	if (!s_FbxSdkManager)
	{
		s_FbxSdkManager = FbxManager::Create();

		FbxIOSettings* ios = FbxIOSettings::Create(s_FbxSdkManager, IOSROOT);
		s_FbxSdkManager->SetIOSettings(ios);

		//Load plugins from the executable directory (optional)
		FbxString path = FbxGetApplicationDirectory();
		s_FbxSdkManager->LoadPluginsDirectory(path.Buffer());

		//Create an FBX scene. This object holds most objects imported/exported from/to files.
		s_FbxScene = FbxScene::Create(s_FbxSdkManager, "FbxScene");

	}

	s_SkeletalIndex = -1;
	s_Bones.clear();

	int fileFormat = -1;
	auto fbxImporter = FbxImporter::Create(s_FbxSdkManager, "");
	if (!s_FbxSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(path.c_str(), fileFormat))
	{
		// 未能识别文件格式
		return false;
	}

	// 初始化Importer，设置文件路径
	if (!fbxImporter->Initialize(path.c_str(), fileFormat))
	{
		return false;
	}

	// 将FBX文件解析导入到Scene中
	if (!fbxImporter->Import(s_FbxScene))
	{
		LOG("Call to FbxImporter::Import() failed.Error reported: %s", fbxImporter->GetStatus().GetErrorString());
		return false;
	}

	// 检查Scene完整性
	FbxStatus status;
	FbxArray<FbxString*> details;
	FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(s_FbxScene), &status, &details);
	//输出错误信息
	if ((!sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData) && details.GetCount() > 0) ||
		(fbxImporter->GetStatus().GetCode() != FbxStatus::eSuccess))
	{
		return false;
	}

	// 转换坐标系
	FbxAxisSystem sceneAxisSystem = s_FbxScene->GetGlobalSettings().GetAxisSystem();
	FbxAxisSystem ourAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);
	if (sceneAxisSystem != ourAxisSystem)
	{
		ourAxisSystem.ConvertScene(s_FbxScene);
	}

	// 转换单元长度
	FbxSystemUnit sceneSystemUnit = s_FbxScene->GetGlobalSettings().GetSystemUnit();
	if (sceneSystemUnit.GetScaleFactor() != 100.0)
	{
		// 例子中用的是厘米，所以这里也要转换
		FbxSystemUnit::cm.ConvertScene(s_FbxScene);
	}

	// 转换曲面到三角形
	FbxGeometryConverter geomConverter(s_FbxSdkManager);
	try
	{
		geomConverter.Triangulate(s_FbxScene, /*replace*/true);
	}
	catch (std::runtime_error)
	{
		LOG("Scene integrity verification failed.");
		return false;
	}

	// 递归解析节点
	Fbx_ParseNode(s_FbxScene->GetRootNode(), skeletalMeshData);
	LOG("extra mesh success");

	fbxImporter->Destroy();
	return true;
#endif // USE_ASSIMP
}

void Fbx_ParseMesh(const FbxMesh* mesh, SkeletalMeshData* skeletalMeshData)
{
	if (!mesh->GetNode())
	{
		LOG("parse fbx mesh node is null!");
		return;
	}

	const int polygonCount = mesh->GetPolygonCount();

	//解析材质
	FbxLayerElementArrayTemplate<int>* materialIndice = NULL;
	FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;
	if (mesh->GetElementMaterial())
	{
		materialIndice = &mesh->GetElementMaterial()->GetIndexArray();
		materialMappingMode = mesh->GetElementMaterial()->GetMappingMode();
		if (materialIndice && materialMappingMode == FbxGeometryElement::eByPolygon)
		{
			FBX_ASSERT(materialIndice->GetCount() == polygonCount);
			if (materialIndice->GetCount() == polygonCount)
			{
				// Count the faces of each material
				for (int i = 0; i < materialIndice->GetCount(); i++)
				{
					const int materialIndex = materialIndice->GetAt(i);
					if (skeletalMeshData->Subsets.size() < materialIndex + 1)
					{
						skeletalMeshData->Subsets.resize(materialIndex + 1);
					}
				}

				// Make sure we have no "holes" (NULL) in the mSubMeshes table. This can happen
				// if, in the loop above, we resized the mSubMeshes by more than one slot.
				/*for (int i = 0; i < mSubMeshes.GetCount(); i++)
				{
					if (mSubMeshes[i] == NULL)
						mSubMeshes[i] = new SubMesh;
				}*/

				// Record the offset (how many vertex)
				//const int lMaterialCount = mSubMeshes.GetCount();
				//int lOffset = 0;
				//for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
				//{
				//	mSubMeshes[lIndex]->IndexOffset = lOffset;
				//	lOffset += mSubMeshes[lIndex]->TriangleCount * 3;
				//	// This will be used as counter in the following procedures, reset to zero
				//	mSubMeshes[lIndex]->TriangleCount = 0;
				//}
				//FBX_ASSERT(lOffset == lPolygonCount * 3);
			}
		}
	}


	// All faces will use the same material.
	if (skeletalMeshData->Subsets.size() == 0)
	{
		skeletalMeshData->Subsets.resize(1);
		skeletalMeshData->Subsets[0].Id = 0;
	}

	// Congregate all the data of a mesh to be cached in VBOs.
	// If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
	bool allByControlPoint = true;
	bool hasNormal = mesh->GetElementNormalCount() > 0;
	bool hasUV = mesh->GetElementUVCount() > 0;

	FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
	FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
	if (hasNormal)
	{
		lNormalMappingMode = mesh->GetElementNormal(0)->GetMappingMode();
		if (lNormalMappingMode == FbxGeometryElement::eNone)
		{
			hasNormal = false;
		}
		if (hasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
		{
			allByControlPoint = false;
		}
	}
	if (hasUV)
	{
		lUVMappingMode = mesh->GetElementUV(0)->GetMappingMode();
		if (lUVMappingMode == FbxGeometryElement::eNone)
		{
			hasUV = false;
		}
		if (hasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
		{
			allByControlPoint = false;
		}
	}

	// Allocate the array memory, by control point or by polygon vertex.
	int polygonVertexCount = mesh->GetControlPointsCount();
	if (!allByControlPoint)
	{
		polygonVertexCount = polygonCount * 3;
	}

	skeletalMeshData->Vertices.resize(polygonVertexCount);
	skeletalMeshData->Indices.resize(polygonCount * 3);
	
	FbxStringList uvNames;
	mesh->GetUVSetNames(uvNames);
	const char* uvName = nullptr;
	if (hasUV && uvNames.GetCount())
	{
		uvName = uvNames[0];
	}

	// Populate the array with vertex attribute, if by control point.
	const FbxVector4* controlPoints = mesh->GetControlPoints();
	FbxVector4 currentVertex;
	FbxVector4 currentNormal;
	FbxVector2 currentUV;
	if (allByControlPoint)
	{
		const FbxGeometryElementNormal* normalElement = NULL;
		const FbxGeometryElementTangent* tangentElement = NULL;
		const FbxGeometryElementUV* lUVElement = NULL;
		if (hasNormal)
		{
			normalElement = mesh->GetElementNormal(0);
			tangentElement = mesh->GetElementTangent(0);
		}

		if (hasUV)
		{
			lUVElement = mesh->GetElementUV(0);
		}
		
		for (int index = 0; index < polygonVertexCount; ++index)
		{
			currentVertex = controlPoints[index];
			skeletalMeshData->Vertices[index].Pos.x = static_cast<float>(currentVertex[0]);
			skeletalMeshData->Vertices[index].Pos.y = static_cast<float>(currentVertex[1]);
			skeletalMeshData->Vertices[index].Pos.z = static_cast<float>(currentVertex[2]);

			if (hasNormal)
			{
				int lNormalIndex = index;
				if (normalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
				{
					lNormalIndex = normalElement->GetIndexArray().GetAt(index);
				}
				currentNormal = normalElement->GetDirectArray().GetAt(lNormalIndex);
				skeletalMeshData->Vertices[index].Normal.x = static_cast<float>(currentNormal[0]);
				skeletalMeshData->Vertices[index].Normal.y = static_cast<float>(currentNormal[1]);
				skeletalMeshData->Vertices[index].Normal.z = static_cast<float>(currentNormal[2]);
			}

			if (hasUV)
			{
				int lUVIndex = index;
				if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
				{
					lUVIndex = lUVElement->GetIndexArray().GetAt(index);
				}
				currentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
				skeletalMeshData->Vertices[index].TexC.x = static_cast<float>(currentUV[0]);
				skeletalMeshData->Vertices[index].TexC.y = 1 - static_cast<float>(currentUV[1]); // 左下角为原点，但是directX、Vulkan贴图坐上角为原点
			}
		}

	}

	uint32 vertexCount = 0;
	for (int i = 0; i < polygonCount; i++)
	{
		// The material for current face.
		int materialIndex = 0;
		if (materialIndice && materialMappingMode == FbxGeometryElement::eByPolygon)
		{
			materialIndex = materialIndice->GetAt(i);
		}

		// Where should I save the vertex attribute index, according to the material
		const uint32 indexOffset = skeletalMeshData->Subsets[materialIndex].IndexStart + skeletalMeshData->Subsets[materialIndex].IndexCount;

		for (int j = 0; j < 3; j++)
		{
			int controlPointIndex = mesh->GetPolygonVertex(i, j);
			// If the lControlPointIndex is -1, we probably have a corrupted mesh data. At this point,
			// it is not guaranteed that the cache will work as expected.

			if (controlPointIndex >= 0)
			{
				if (allByControlPoint)
				{
					skeletalMeshData->Indices[indexOffset + j] = static_cast<unsigned int>(controlPointIndex);
				}
				// Populate the array with vertex attribute, if by polygon vertex.
				else
				{
					currentVertex = controlPoints[controlPointIndex];
					controlPointIndex = vertexCount;
					skeletalMeshData->Indices[indexOffset + j] = controlPointIndex;//static_cast<unsigned int>(vertexCount);

					skeletalMeshData->Vertices[controlPointIndex].Pos.x = static_cast<float>(currentVertex[0]);
					skeletalMeshData->Vertices[controlPointIndex].Pos.y = static_cast<float>(currentVertex[1]);
					skeletalMeshData->Vertices[controlPointIndex].Pos.z = static_cast<float>(currentVertex[2]);

					if (hasNormal)
					{
						mesh->GetPolygonVertexNormal(i, j, currentNormal);
						skeletalMeshData->Vertices[controlPointIndex].Normal.x = static_cast<float>(currentNormal[0]);
						skeletalMeshData->Vertices[controlPointIndex].Normal.y = static_cast<float>(currentNormal[1]);
						skeletalMeshData->Vertices[controlPointIndex].Normal.z = static_cast<float>(currentNormal[2]);
					}

					if (hasUV)
					{
						bool bUnmappedUV;
						mesh->GetPolygonVertexUV(i, j, uvName, currentUV, bUnmappedUV);
						skeletalMeshData->Vertices[controlPointIndex].TexC.x = static_cast<float>(currentUV[0]);
						skeletalMeshData->Vertices[controlPointIndex].TexC.y = 1 - static_cast<float>(currentUV[1]);
					}

					vertexCount++;
				}
			}
		}

		skeletalMeshData->Subsets[materialIndex].IndexCount += 3;
	}

	skeletalMeshData->Subsets[0].VertexCount = (uint32)skeletalMeshData->Vertices.size();
	auto shader = AssetManager::Get()->GetShader("skeletalOpaque");//
	auto mat = Material::CreateMaterial("womenMat");
	mat->MatCBIndex = 1;
	mat->m_Shader = shader;
	mat->m_DiffuseTexture = AssetManager::Get()->GetTexture("jiulian");
	mat->m_NormalTexture = AssetManager::Get()->GetTexture("bricksNormalMap");
	skeletalMeshData->Materials.push_back(mat);

	// 获取蒙皮修改器
	FbxSkin* skinDeformer = (FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin);

	// 获取蒙皮修改器上的顶点组数量，一般来说就是骨骼数量，绑定的时候，一般是以一个骨骼作为一个顶点组。
	int clusterCount = skinDeformer->GetClusterCount();

	// 遍历骨骼
	V_Array<VertexRelateBoneInfo> vertex_relate_bone_infos_(skeletalMeshData->Vertices.size());

	for (int lClusterIndex = 0; lClusterIndex < clusterCount; ++lClusterIndex)
	{
		// 获取骨骼的顶点组
		FbxCluster* cluster = skinDeformer->GetCluster(lClusterIndex);
		auto linkBone = cluster->GetLink();
		auto name = linkBone->GetName();

		int boneIndex = -1;
		for (int i = 0; i < s_Bones.size(); i++)
		{
			if (s_Bones[i].first == name)
			{
				boneIndex = i;
				s_Bones[i].second.second = cluster;

				const FbxVector4 lT = s_Bones[i].second.first->GetGeometricTranslation(FbxNode::eSourcePivot);
				const FbxVector4 lR = s_Bones[i].second.first->GetGeometricRotation(FbxNode::eSourcePivot);
				const FbxVector4 lS = s_Bones[i].second.first->GetGeometricScaling(FbxNode::eSourcePivot);

				FbxAMatrix geometryTransform = FbxAMatrix(lT, lR, lS);

				FbxAMatrix transformMatrix, transformLinkMatrix;
				s_Bones[i].second.second->GetTransformMatrix(transformMatrix);
				s_Bones[i].second.second->GetTransformLinkMatrix(transformLinkMatrix);
				auto globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

				Matrix4x4 transform((double(*)[4])(&globalBindposeInverseMatrix.mData));
				for (int k = 0; k < 4; k++)
				{
					for (int j = 0; j < 4; j++)
					{
						transform.m[k][j] = (float)globalBindposeInverseMatrix.Get(k, j);
					}
				}
				skeletalMeshData->BoneOffsets[i] = transform;

				globalBindposeInverseMatrix = transformMatrix.Inverse();
				for (int k = 0; k < 4; k++)
				{
					for (int j = 0; j < 4; j++)
					{
						transform.m[k][j] = (float)globalBindposeInverseMatrix.Get(k, j);
					}
				}
				skeletalMeshData->BoneOffsets1[i] = transform;
				break;
			}
		}

		// 获取这个顶点组影响的顶点索引数量
		int lVertexIndexCount = cluster->GetControlPointIndicesCount();
		for (int k = 0; k < lVertexIndexCount; ++k)
		{
			//拿到顶点索引
			int vertexIndex = cluster->GetControlPointIndices()[k];

			//拿到这个簇中对这个顶点的权重
			auto weight = cluster->GetControlPointWeights()[k];
			vertex_relate_bone_infos_[vertexIndex].Push(boneIndex, (int)(weight * 100));
		}
	}

	for (size_t i = 0; i < s_Bones.size(); i++)
	{
		if (!s_Bones[i].second.second)
		{
			memset(&skeletalMeshData->BoneOffsets[i], 0, sizeof(skeletalMeshData->BoneOffsets[i]));
		}
	}
	
	// 遍历所有三角面，遍历每个面的三个顶点，解析顶点坐标、UV坐标数据。
	vertexCount = 0;
	for (int lPolygonIndex = 0; lPolygonIndex < polygonCount; ++lPolygonIndex)
	{
		// 三角面，3个顶点
		for (int lVerticeIndex = 0; lVerticeIndex < 3; ++lVerticeIndex)
		{
			// 传入面索引，以及当前面的第几个顶点，获取顶点索引。
			const int lControlPointIndex = mesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
			if (lControlPointIndex >= 0)
			{
				VertexRelateBoneInfo vertex_relate_bone_info = vertex_relate_bone_infos_[lControlPointIndex];
				for (int i = 0; i < 4; ++i)
				{
					char bone_index = vertex_relate_bone_info.bone_index_[i];
					char weight = vertex_relate_bone_info.bone_weight_[i];
					if (allByControlPoint)
					{
						skeletalMeshData->Vertices[lControlPointIndex].BoneIndices[i] = bone_index < 0 ? 0 : bone_index;
						if (i < 3)
						{
							skeletalMeshData->Vertices[lControlPointIndex].BoneWeights[i] = weight / 100.f;
						}
					}
					else
					{
						skeletalMeshData->Vertices[vertexCount].BoneIndices[i] = bone_index < 0 ? 0 : bone_index;
						if (i < 3)
						{
							skeletalMeshData->Vertices[vertexCount].BoneWeights[i] = weight / 100.f;
						}
					}
				}

				vertexCount++;
			}
		}
	}
}

void Fbx_ParseNode(FbxNode* node, SkeletalMeshData* skeletalMeshData, int parentSkeletalIndex)
{
	FbxNodeAttribute* nodeAttribute = node->GetNodeAttribute();
	if (nodeAttribute)
	{
		auto type = nodeAttribute->GetAttributeType();
		if (type == FbxNodeAttribute::eMesh)
		{
			FbxMesh* lMesh = node->GetMesh();
			if (lMesh && !lMesh->GetUserDataPtr())
			{
				Fbx_ParseMesh(lMesh, skeletalMeshData);
			}

			Fbx_ParseAniamtion(node, skeletalMeshData);
			//WriteToFile(skeletalMeshData);
		}
		else if (type == FbxNodeAttribute::eSkeleton)
		{
			skeletalMeshData->BoneHierarchy.push_back(parentSkeletalIndex);
			skeletalMeshData->BoneOffsets.push_back(Matrix4x4::UnitMatrix());
			skeletalMeshData->BoneOffsets1.push_back(Matrix4x4::UnitMatrix());
			s_Bones.push_back({ node->GetName(),  { node, nullptr } });

			s_SkeletalIndex++;
		}
	}

	// 递归解析子节点
	const int childCount = node->GetChildCount();
	auto parentIndex = s_SkeletalIndex;
	for (int i = 0; i < childCount; ++i)
	{
		Fbx_ParseNode(node->GetChild(i), skeletalMeshData, parentIndex);
	}
}

void Fbx_ParseAniamtion(FbxNode* pNode, SkeletalMeshData* skeletalMeshData)
{
	auto name = pNode->GetName();
	LOG("node name: %s", name);
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
	if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		// 获取pNode相对于锚点的offset
		const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
		FbxAMatrix geometryOffset = FbxAMatrix(lT, lR, lS);
		
		FbxMesh* pMesh = pNode->GetMesh();

		FbxArray<FbxString*>animStackNameArray;
		s_FbxScene->FillAnimStackNameArray(animStackNameArray);
		for (int i = 0; i < animStackNameArray.Size(); i++)
		{
			auto animBuffer = s_FbxScene->FindMember<FbxAnimStack>(animStackNameArray[i]->Buffer());
			s_FbxScene->SetCurrentAnimationStack(animBuffer);

			// 获取动画片段的时间范围
			FbxTime mStart, mStop;
			FbxTakeInfo* lCurrentTakeInfo = s_FbxScene->GetTakeInfo(*(animStackNameArray[i]));
			if (lCurrentTakeInfo)
			{
				mStart = lCurrentTakeInfo->mLocalTimeSpan.GetStart();
				mStop = lCurrentTakeInfo->mLocalTimeSpan.GetStop();
			}

			AnimationClip animationClip;
			FbxTime::EMode lTimeMode = s_FbxScene->GetGlobalSettings().GetTimeMode();
			//animation.frame_per_second_ = fbxsdk::FbxTime::GetFrameRate(lTimeMode);

			// 每一帧的时间
			FbxTime mFrameTime;
			mFrameTime.SetTime(0, 0, 0, 1, 0, s_FbxScene->GetGlobalSettings().GetTimeMode());
			int lSkinIndex = 0;
			FbxSkin* lSkinDeformer = (FbxSkin*)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);

			animationClip.BoneAnimations.resize(s_Bones.size());
			
			for (FbxTime pTime = mStart; pTime < mStop; pTime += mFrameTime)
			{
				FbxAMatrix globalPos;
				globalPos = pNode->EvaluateLocalTransform(pTime); //世界原点的Transform，单位厘米
				for (size_t i = 0; i < s_Bones.size(); i++)
				{
					Keyframe frame;
					frame.TimePos = (float)pTime.GetSecondDouble();

					auto boneLocalTransform = s_Bones[i].second.first->EvaluateLocalTransform(pTime);
					if (s_Bones[i].second.second) 
					{
						auto parent = s_Bones[i].second.first->GetParent();
						if (parent->GetNodeAttribute() && parent->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eNull)
						{
							boneLocalTransform = parent->EvaluateLocalTransform(pTime) * boneLocalTransform;
						}
					}
					else
					{
						
					}

					auto t = boneLocalTransform.GetT();
					auto rq = boneLocalTransform.GetQ();
					auto s = boneLocalTransform.GetS();

					frame.Translation.x = (float)t.mData[0];
					frame.Translation.y = (float)t.mData[1];
					frame.Translation.z = (float)t.mData[2];

					frame.RotationQuat.x = (float)rq.mData[0];
					frame.RotationQuat.y = (float)rq.mData[1];
					frame.RotationQuat.z = (float)rq.mData[2];
					frame.RotationQuat.w = (float)rq.mData[3];

					frame.Scale.x = (float)s.mData[0];
					frame.Scale.y = (float)s.mData[1];
					frame.Scale.z = (float)s.mData[2];
					
					animationClip.BoneAnimations[i].Keyframes.push_back(frame);
				}
			}

			skeletalMeshData->AnimClips["anim" + std::to_string(i)] = animationClip;
		}
	}
}