#include "RiverPch.h"
#include "Renderer/Mesh/Header/Mesh.h"
#include "Renderer/Header/AssetManager.h"

#include "fbxsdk/core/fbxmanager.h"
#include "fbxsdk/core/math/fbxvector4.h"
#include "fbxsdk.h"

#include <fstream>

#include "Math/Header/Geometric.h"

bool LoadFbxMesh(const String& path, SkeletalMeshData* skeletalMeshData);
void ParseMesh(const FbxMesh* pMesh, SkeletalMeshData* skeletalMeshData);
void ParseNode(FbxNode* pNode, SkeletalMeshData* skeletalMeshData, int parentSkeletalIndex = -1);
void ParseAniamtion(FbxNode* pNode, SkeletalMeshData* skeletalMeshData);

static FbxManager* s_FbxSdkManager = nullptr;
static FbxScene* s_FbxScene = nullptr;
static int s_SkeletalIndex = -1;
static V_Array<Pair<String, FbxNode*<< s_Bones;

struct VertexRelateBoneInfo {
	char bone_index_[4];//骨骼索引，一般骨骼少于128个，用char就行。
	char bone_weight_[4];//骨骼权重，权重不会超过100，所以用char类型就可以。
	VertexRelateBoneInfo() {
		for (int i = 0; i < sizeof(bone_index_); ++i) {
			bone_index_[i] = -1;
		}
		for (int i = 0; i < sizeof(bone_weight_); ++i) {
			bone_weight_[i] = 0;
		}
	}
	void Push(char bone_index, char bone_weight) {
		for (int i = 0; i < sizeof(bone_index_); ++i) {
			if (bone_index_[i] == -1) {
				bone_index_[i] = bone_index;
				if (bone_weight_[i] == 0) {
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

void ComputeClusterDeformation(FbxAMatrix& pGlobalPosition, FbxMesh* pMesh, FbxCluster* pCluster, FbxAMatrix& pVertexTransformMatrix, FbxTime pTime)
{
	FbxCluster::ELinkMode lClusterMode = pCluster->GetLinkMode();

	if (lClusterMode == FbxCluster::eNormalize)
	{
		FbxAMatrix lReferenceGlobalInitPosition;
		FbxAMatrix lAssociateGlobalInitPosition;
		FbxAMatrix lAssociateGlobalCurrentPosition;
		FbxAMatrix lClusterGlobalInitPosition;
		FbxAMatrix lClusterGlobalCurrentPosition;

		FbxAMatrix lAssociateGeometry;
		FbxAMatrix lClusterGeometry;

		FbxAMatrix lClusterRelativeInitPosition;
		FbxAMatrix lClusterRelativeCurrentPositionInverse;
		pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
		// Multiply lReferenceGlobalInitPosition by Geometric Transformation
		const FbxVector4 lT = pMesh->GetNode()->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 lR = pMesh->GetNode()->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 lS = pMesh->GetNode()->GetGeometricScaling(FbxNode::eSourcePivot);
		FbxAMatrix lReferenceGeometry = FbxAMatrix(lT, lR, lS);

		lReferenceGlobalInitPosition *= lReferenceGeometry;

		// Get the link initial global position and the link current global position.
		pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);

		lClusterGlobalCurrentPosition = pCluster->GetLink()->EvaluateGlobalTransform(pTime);

		// Compute the initial position of the link relative to the reference.
		lClusterRelativeInitPosition = lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;

		// Compute the current position of the link relative to the reference.
		lClusterRelativeCurrentPositionInverse = pGlobalPosition.Inverse() * lClusterGlobalCurrentPosition;

		// Compute the shift of the link relative to the reference.
		pVertexTransformMatrix = lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
	}
}

bool LoadFbxMesh(const String& path, SkeletalMeshData* skeletalMeshData)
{
	if (!s_FbxSdkManager)
	{
		s_FbxSdkManager = FbxManager::Create();

		FbxIOSettings* ios = FbxIOSettings::Create(s_FbxSdkManager, IOSROOT);
		s_FbxSdkManager->SetIOSettings(ios);

		//Load plugins from the executable directory (optional)
		FbxString lPath = FbxGetApplicationDirectory();
		s_FbxSdkManager->LoadPluginsDirectory(lPath.Buffer());

		//Create an FBX scene. This object holds most objects imported/exported from/to files.
		s_FbxScene = FbxScene::Create(s_FbxSdkManager, "My Scene");

	}

	s_SkeletalIndex = -1;
	s_Bones.clear();

	int lFileFormat = -1;
	auto fbxImporter = FbxImporter::Create(s_FbxSdkManager, "");
	if (!s_FbxSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(path.c_str(), lFileFormat))
	{
		// 未能识别文件格式
		return false;
	}

	// 初始化Importer，设置文件路径
	if (!fbxImporter->Initialize(path.c_str(), lFileFormat))
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

	// 转换坐标系为右手坐标系。
	FbxAxisSystem SceneAxisSystem = s_FbxScene->GetGlobalSettings().GetAxisSystem();
	FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
	if (SceneAxisSystem != OurAxisSystem)
	{
		OurAxisSystem.ConvertScene(s_FbxScene);
	}

	// 转换单元长度
	FbxSystemUnit SceneSystemUnit = s_FbxScene->GetGlobalSettings().GetSystemUnit();
	if (SceneSystemUnit.GetScaleFactor() != 1.0)
	{
		// 例子中用的是厘米，所以这里也要转换
		FbxSystemUnit::cm.ConvertScene(s_FbxScene);
	}

	// 转换曲面到三角形
	FbxGeometryConverter lGeomConverter(s_FbxSdkManager);
	try
	{
		lGeomConverter.Triangulate(s_FbxScene, /*replace*/true);
	}
	catch (std::runtime_error)
	{
		LOG("Scene integrity verification failed.\n");
		return false;
	}

	// 递归解析节点
	ParseNode(s_FbxScene->GetRootNode(), skeletalMeshData);
	LOG("extra mesh success");

	fbxImporter->Destroy();
}

void ParseMesh(const FbxMesh* pMesh, SkeletalMeshData* skeletalMeshData)
{
	if (!pMesh->GetNode())
	{
		LOG("parse fbx mesh node is null!");
		return;
	}

	const int polygonCount = pMesh->GetPolygonCount();

	//解析材质
	FbxLayerElementArrayTemplate<int>* materialIndice = NULL;
	FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;
	if (pMesh->GetElementMaterial())
	{
		materialIndice = &pMesh->GetElementMaterial()->GetIndexArray();
		materialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
		if (materialIndice && materialMappingMode == FbxGeometryElement::eByPolygon)
		{
			FBX_ASSERT(materialIndice->GetCount() == polygonCount);
			if (materialIndice->GetCount() == polygonCount)
			{
				// Count the faces of each material
				for (int i = 0; i < polygonCount; i++)
				{
					const int materialIndex = materialIndice->GetAt(i);
					if (skeletalMeshData->Subsets.size() < materialIndex + 1)
					{
						skeletalMeshData->Subsets.resize(materialIndex + 1);
					}
					/*if (skeletalMeshData->Subsets[materialIndex] == nullptr)
					{
						mSubMeshes[lMaterialIndex] = new SubMesh;
					}
					mSubMeshes[lMaterialIndex]->TriangleCount += 1;*/
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
	}

	// Congregate all the data of a mesh to be cached in VBOs.
	// If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
	bool allByControlPoint = true;
	bool hasNormal = pMesh->GetElementNormalCount() > 0;
	bool hasUV = pMesh->GetElementUVCount() > 0;

	FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
	FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
	if (hasNormal)
	{
		lNormalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
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
		lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
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
	allByControlPoint = true;
	int polygonVertexCount = pMesh->GetControlPointsCount();
	if (!allByControlPoint)
	{
		polygonVertexCount = polygonCount * 3;
	}

	skeletalMeshData->Vertices.resize(polygonVertexCount);
	skeletalMeshData->Indices.resize(polygonCount * 3);
	
	FbxStringList lUVNames;
	pMesh->GetUVSetNames(lUVNames);
	const char* lUVName = NULL;
	if (hasUV && lUVNames.GetCount())
	{
		lUVName = lUVNames[0];
	}

	// Populate the array with vertex attribute, if by control point.
	const FbxVector4* controlPoints = pMesh->GetControlPoints();
	FbxVector4 currentVertex;
	FbxVector4 currentNormal;
	FbxVector2 currentUV;
	if (allByControlPoint)
	{
		const FbxGeometryElementNormal* normalElement = NULL;
		const FbxGeometryElementUV* lUVElement = NULL;
		if (hasNormal)
		{
			normalElement = pMesh->GetElementNormal(0);
		}
		if (hasUV)
		{
			lUVElement = pMesh->GetElementUV(0);
		}
		for (int index = 0; index < polygonVertexCount; ++index)
		{
			// Save the vertex position.
			currentVertex = controlPoints[index];
			skeletalMeshData->Vertices[index].Pos.x = static_cast<float>(currentVertex[0]);
			skeletalMeshData->Vertices[index].Pos.y = static_cast<float>(currentVertex[1]);
			skeletalMeshData->Vertices[index].Pos.z = static_cast<float>(currentVertex[2]);

			// Save the normal.
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

			// Save the UV.
			if (hasUV)
			{
				int lUVIndex = index;
				if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
				{
					lUVIndex = lUVElement->GetIndexArray().GetAt(index);
				}
				currentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
				skeletalMeshData->Vertices[index].TexC.x = static_cast<float>(currentUV[0]);
				skeletalMeshData->Vertices[index].TexC.y = static_cast<float>(currentUV[1]);
			}
		}

	}

	int vertexCount = 0;
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
			const int controlPointIndex = pMesh->GetPolygonVertex(i, j);
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
					skeletalMeshData->Indices[indexOffset + j] = controlPointIndex;//static_cast<unsigned int>(vertexCount);

					currentVertex = controlPoints[controlPointIndex];
					skeletalMeshData->Vertices[controlPointIndex].Pos.x = static_cast<float>(currentVertex[0]);
					skeletalMeshData->Vertices[controlPointIndex].Pos.y = static_cast<float>(currentVertex[1]);
					skeletalMeshData->Vertices[controlPointIndex].Pos.z = static_cast<float>(currentVertex[2]);

					if (hasNormal)
					{
						pMesh->GetPolygonVertexNormal(i, j, currentNormal);
						skeletalMeshData->Vertices[controlPointIndex].Normal.x = static_cast<float>(currentNormal[0]);
						skeletalMeshData->Vertices[controlPointIndex].Normal.y = static_cast<float>(currentNormal[1]);
						skeletalMeshData->Vertices[controlPointIndex].Normal.z = static_cast<float>(currentNormal[2]);
					}

					if (hasUV)
					{
						bool bUnmappedUV;
						pMesh->GetPolygonVertexUV(i, j, lUVName, currentUV, bUnmappedUV);
						skeletalMeshData->Vertices[controlPointIndex].TexC.x = static_cast<float>(currentUV[0]);
						skeletalMeshData->Vertices[controlPointIndex].TexC.y = static_cast<float>(currentUV[1]);
					}
				}
			}
			++vertexCount;
			LOG("point %d %d", controlPointIndex, vertexCount);
		}

		skeletalMeshData->Subsets[materialIndex].IndexCount += 3;
	}

	auto shader = AssetManager::Get()->GetShader("opaque");//skeletalO
	auto mat = Material::CreateMaterial("womenMat");
	mat->MatCBIndex = 1;
	mat->m_Shader = shader;
	skeletalMeshData->Materials.push_back(mat);

	// 获取蒙皮修改器
	int lSkinIndex = 0;
	FbxSkin* lSkinDeformer = (FbxSkin*)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
	// 获取蒙皮修改器上的顶点组数量，一般来说就是骨骼数量，绑定的时候，一般是以一个骨骼作为一个顶点组。
	int lClusterCount = lSkinDeformer->GetClusterCount();

	// 遍历骨骼
	V_Array<VertexRelateBoneInfo> vertex_relate_bone_infos_(vertexCount);

	//skeletalMeshData->BoneHierarchy.resize(lClusterCount);
	for (int lClusterIndex = 0; lClusterIndex < lClusterCount; ++lClusterIndex)
	{
		// 获取骨骼的顶点组
		/*FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
		
		animation.bone_name_vec_.push_back(lCluster->GetName());*/

		// 获取骨骼的顶点组
		FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
		// 获取这个顶点组影响的顶点索引数量
		int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
		for (int k = 0; k < lVertexIndexCount; ++k)
		{
			//拿到顶点索引
			int lIndex = lCluster->GetControlPointIndices()[k];
			//拿到这个簇中对这个顶点的权重
			auto lWeight = lCluster->GetControlPointWeights()[k];
			vertex_relate_bone_infos_[lIndex].Push(lClusterIndex, (int)(lWeight * 100));
			LOG("weight %d %d %f", lIndex, lClusterIndex, lWeight);
		}
	}
	
	// 遍历所有三角面，遍历每个面的三个顶点，解析顶点坐标、UV坐标数据。
	for (int lPolygonIndex = 0; lPolygonIndex < polygonCount; ++lPolygonIndex)
	{
		// 三角面，3个顶点
		for (int lVerticeIndex = 0; lVerticeIndex < 3; ++lVerticeIndex)
		{
			// 传入面索引，以及当前面的第几个顶点，获取顶点索引。
			const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
			if (lControlPointIndex >= 0)
			{
				VertexRelateBoneInfo vertex_relate_bone_info = vertex_relate_bone_infos_[lControlPointIndex];
				for (int i = 0; i < 4; ++i)
				{
					char bone_index = vertex_relate_bone_info.bone_index_[i];
					char weight = vertex_relate_bone_info.bone_weight_[i];
					skeletalMeshData->Vertices[lControlPointIndex].BoneIndices[i] = bone_index < 0 ? 0 : bone_index;
					if (i < 3)
					{
						skeletalMeshData->Vertices[lControlPointIndex].BoneWeights[i] = weight/100.f;
					}
				}
			}
		}
	}

	LOG("");
	// 写入文件
	std::ofstream meshFile("F:\\GitHub\\River\\River\\Models\\women.m3d");

	meshFile << "***************m3d-File-Header*******************" << std::endl;
	meshFile << "#Materials " << skeletalMeshData->Materials.size() << std::endl;
	meshFile << "#Vertices " << skeletalMeshData->Vertices.size() << std::endl;
	meshFile << "#Triangles " << skeletalMeshData->Indices.size() / 3 << std::endl;
	meshFile << "#Bones " << skeletalMeshData->BoneHierarchy.size() << std::endl;
	meshFile << "#AnimationClips " << 0 << std::endl << std::endl;

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
		meshFile << "MaterialTypeName: " << "opaque" << std::endl;
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

	meshFile << "***************BoneOffsets*******************" << std::endl;
	for (size_t i = 0; i < skeletalMeshData->BoneOffsets.size(); i++)
	{
		meshFile<<"BoneOffset"<<i << " "<<
			skeletalMeshData->BoneOffsets[i](0, 0) << skeletalMeshData->BoneOffsets[i](0, 1) << skeletalMeshData->BoneOffsets[i](0, 2) << skeletalMeshData->BoneOffsets[i](0, 3) <<
			skeletalMeshData->BoneOffsets[i](1, 0) << skeletalMeshData->BoneOffsets[i](1, 1) << skeletalMeshData->BoneOffsets[i](1, 2) << skeletalMeshData->BoneOffsets[i](1, 3) <<
			skeletalMeshData->BoneOffsets[i](2, 0) << skeletalMeshData->BoneOffsets[i](2, 1) << skeletalMeshData->BoneOffsets[i](2, 2) << skeletalMeshData->BoneOffsets[i](2, 3) <<
			skeletalMeshData->BoneOffsets[i](3, 0) << skeletalMeshData->BoneOffsets[i](3, 1) << skeletalMeshData->BoneOffsets[i](3, 2) << skeletalMeshData->BoneOffsets[i](3, 3) <<
		std::endl;
	}

	meshFile << "***************BoneHierarchy*****************" << std::endl;
	for (size_t i = 0; i < skeletalMeshData->BoneHierarchy.size(); i++)
	{

	}
}

void ParseNode(FbxNode* pNode, SkeletalMeshData* skeletalMeshData, int parentSkeletalIndex)
{
	// 获取节点属性
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
	if (lNodeAttribute)
	{
		// 节点是Mesh
		auto type = lNodeAttribute->GetAttributeType();
		if (type == FbxNodeAttribute::eMesh)
		{
			FbxMesh* lMesh = pNode->GetMesh();
			if (lMesh && !lMesh->GetUserDataPtr())
			{
				ParseMesh(lMesh, skeletalMeshData);
			}

			ParseAniamtion(pNode, skeletalMeshData);
		}
		else if (type == FbxNodeAttribute::eSkeleton)
		{
			skeletalMeshData->BoneHierarchy.push_back(parentSkeletalIndex);

			Transform transform;
			auto t = pNode->LclTranslation.Get();
			transform.Position = { (float)t.mData[0], (float)t.mData[2], (float)t.mData[1] };

			auto r = pNode->LclRotation.Get();
			transform.Rotation = { (float)r.mData[0], (float)r.mData[2], (float)r.mData[1] };

			auto s = pNode->LclScaling.Get();
			transform.Scale = { (float)s.mData[0], (float)s.mData[2], (float)s.mData[1] };

			skeletalMeshData->BoneOffsets.push_back(transform);
			s_Bones.push_back({ pNode->GetName(), pNode });

			s_SkeletalIndex++;
		}
		else if (type == FbxNodeAttribute::eNull)
		{
			s_Bones.push_back({ pNode->GetName(), pNode });
		}

		
	}

	// 递归解析子节点
	const int lChildCount = pNode->GetChildCount();

	auto parentIndex = s_SkeletalIndex;
	LOG("%s count %d", pNode->GetName(), lChildCount);
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		ParseNode(pNode->GetChild(lChildIndex), skeletalMeshData, parentIndex);
		LOG("%s     %s", pNode->GetName(), pNode->GetChild(lChildIndex)->GetName());
	}
}

/// 从Mesh节点获取蒙皮修改器，获取所有Cluster。遍历帧数，获取每一帧每个Cluster的移动旋转矩阵数据。
/// \param pNode Mesh节点
void ParseAniamtion(FbxNode* pNode, SkeletalMeshData* skeletalMeshData)
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
		FbxAMatrix lGeometryOffset = FbxAMatrix(lT, lR, lS);
		LOG(" node translation: (%f,%f,%f,%f)", lT[0], lT[1], lT[2], lT[3]);
		LOG(" node rotation: (%f,%f,%f,%f)", lR[0], lR[1], lR[2], lR[3]);
		LOG(" node scaling: (%f,%f,%f,%f)", lS[0], lS[1], lS[2], lS[3]);

		FbxMesh* pMesh = pNode->GetMesh();
		// 获取蒙皮数量，一般来说一个Mesh对应一个蒙皮修改器。
//            int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
			//Engine::Animation animation;
		FbxArray<FbxString*>animStackNameArray;
		s_FbxScene->FillAnimStackNameArray(animStackNameArray);
		for (size_t i = 0; i < animStackNameArray.Size(); i++)
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

			animationClip.BoneAnimations.resize(s_Bones.size() - 1);
			for (FbxTime pTime = mStart; pTime < mStop; pTime += mFrameTime)
			{
				for (size_t i = 0; i < s_Bones.size(); i++)
				{
					Keyframe frame;
					auto t = s_Bones[i].second->EvaluateLocalTranslation(pTime);
					auto r = s_Bones[i].second->EvaluateLocalRotation(pTime);
					auto s = s_Bones[i].second->EvaluateLocalScaling(pTime);

					frame.Translation.x = t.mData[0];
					frame.Translation.y = t.mData[1];
					frame.Translation.z = t.mData[2];

					frame.RotationQuat = VectorToQuaternion({ (float)r.mData[0], (float)r.mData[1], (float)r.mData[2], (float)r.mData[3] });

					frame.Scale.x = s.mData[0];
					frame.Scale.y = s.mData[1];
					frame.Scale.z = s.mData[2];

					frame.TimePos = (float)pTime.GetSecondDouble();
					animationClip.BoneAnimations[i].Keyframes.push_back(frame);
				}
				
				//// 首先获取当前节点的全局坐标
				//FbxAMatrix lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
				//FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;//相乘获得pNode在当前时间相对原点的坐标。

				////V_Array<Matrix4x4> one_frame_bone_matrix_vec;//一帧的所有骨骼变换矩阵
				//auto count = lSkinDeformer->GetClusterCount();
				//for (int lClusterIndex = 0; lClusterIndex < count; ++lClusterIndex)
				//{
				//	Keyframe frame;
				//	frame.TimePos = (float)pTime.GetSecondDouble();

				//	// 获取骨骼的顶点组
				//	FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
				//	
				//	// 计算这个骨骼的形变，前面pNode是指计算到Mesh节点的形变，而这是是计算骨骼节点，后面会作用到顶点。
				//	FbxAMatrix lVertexTransformMatrix;
				//	ComputeClusterDeformation(lGlobalOffPosition, pMesh, lCluster, lVertexTransformMatrix, pTime);
				//	frame.BoneMatrix = FbxMatrixToGlmMat4(lVertexTransformMatrix);


				//	animationClip.BoneAnimations[lClusterIndex].Keyframes.push_back(frame);
				//}

				//animation.frame_bones_matrix_vec_.push_back(one_frame_bone_matrix_vec);
			}//lClusterCount
			//animation.frame_count_ = animation.frame_bones_matrix_vec_.size();
			//animation.Write(fmt::format("../data/animation/fbx_extra_{}.skeleton_anim", animation.name_).c_str());

			skeletalMeshData->AnimClips[animBuffer->GetName()] = animationClip;
		}
	}
}