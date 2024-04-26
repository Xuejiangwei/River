#include "RiverPch.h"
#include "Renderer/Mesh/Header/Mesh.h"
#include "Renderer/Header/AssetManager.h"

#include "fbxsdk/core/fbxmanager.h"
#include "fbxsdk/core/math/fbxvector4.h"
#include "fbxsdk.h"

#include <fstream>

bool LoadFbxMesh(const String& path, SkeletalMeshData* skeletalMeshData);
void ParseMesh(const FbxMesh* pMesh, SkeletalMeshData* skeletalMeshData);
void ParseNode(FbxNode* pNode, SkeletalMeshData* skeletalMeshData, int parentSkeletalIndex = -1);
void ParseAniamtion(FbxNode* pNode, SkeletalMeshData* skeletalMeshData);

static FbxManager* sdkManager = nullptr;
static FbxScene* fbxScene = nullptr;
static FbxImporter* fbxImporter = nullptr;
static int s_SkeletalIndex = -1;

struct VertexRelateBoneInfo {
	char bone_index_[4];//骨骼索引，一般骨骼少于128个，用char就行。
	char bone_weight_[4];//骨骼权重，权重不会超过100，所以用char类型就可以。
	VertexRelateBoneInfo() {
		for (int i = 0; i < sizeof(bone_index_); ++i) {
			bone_index_[i] = -1;
		}
		for (int i = 0; i < sizeof(bone_weight_); ++i) {
			bone_weight_[i] = -1;
		}
	}
	void Push(char bone_index, char bone_weight) {
		for (int i = 0; i < sizeof(bone_index_); ++i) {
			if (bone_index_[i] == -1) {
				bone_index_[i] = bone_index;
				if (bone_weight_[i] == -1) {
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
	if (!sdkManager)
	{
		sdkManager = FbxManager::Create();

		FbxIOSettings* ios = FbxIOSettings::Create(sdkManager, IOSROOT);
		sdkManager->SetIOSettings(ios);

		//Load plugins from the executable directory (optional)
		FbxString lPath = FbxGetApplicationDirectory();
		sdkManager->LoadPluginsDirectory(lPath.Buffer());

		//Create an FBX scene. This object holds most objects imported/exported from/to files.
		fbxScene = FbxScene::Create(sdkManager, "My Scene");

		fbxImporter = FbxImporter::Create(sdkManager, "");
	}

	s_SkeletalIndex = -1;
	int lFileFormat = -1;
	if (!sdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(path.c_str(), lFileFormat))
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
	if (!fbxImporter->Import(fbxScene))
	{
		LOG("Call to FbxImporter::Import() failed.Error reported: %s", fbxImporter->GetStatus().GetErrorString());
		return false;
	}

	// 检查Scene完整性
	FbxStatus status;
	FbxArray<FbxString*> details;
	FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(fbxScene), &status, &details);
	//输出错误信息
	if ((!sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData) && details.GetCount() > 0) ||
		(fbxImporter->GetStatus().GetCode() != FbxStatus::eSuccess))
	{
		return false;
	}

	// 转换坐标系为右手坐标系。
	FbxAxisSystem SceneAxisSystem = fbxScene->GetGlobalSettings().GetAxisSystem();
	FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
	if (SceneAxisSystem != OurAxisSystem)
	{
		OurAxisSystem.ConvertScene(fbxScene);
	}

	// 转换单元长度
	FbxSystemUnit SceneSystemUnit = fbxScene->GetGlobalSettings().GetSystemUnit();
	if (SceneSystemUnit.GetScaleFactor() != 1.0)
	{
		// 例子中用的是厘米，所以这里也要转换
		FbxSystemUnit::cm.ConvertScene(fbxScene);
	}

	// 转换曲面到三角形
	FbxGeometryConverter lGeomConverter(sdkManager);
	try
	{
		lGeomConverter.Triangulate(fbxScene, /*replace*/true);
	}
	catch (std::runtime_error)
	{
		LOG("Scene integrity verification failed.\n");
		return false;
	}

	// 递归解析节点
	ParseNode(fbxScene->GetRootNode(), skeletalMeshData);
	LOG("extra mesh success");
}

void ParseMesh(const FbxMesh* pMesh, SkeletalMeshData* skeletalMeshData)
{
	FbxNode* lNode = pMesh->GetNode();
	if (!lNode)
	{
		LOG("Mesh has no node.");
		return;
	}
	//DEBUG_LOG_INFO("Mesh name: {}", lNode->GetName());
	// 获取Mesh多边形个数，对游戏来说就是三角形面数。
	const int lPolygonCount = pMesh->GetPolygonCount();
	// 是否有UV数据？
	bool mHasUV = pMesh->GetElementUVCount() > 0;
	bool mAllByControlPoint = true;
	FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
	if (mHasUV)
	{
		lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
		if (lUVMappingMode == FbxGeometryElement::eNone)
		{
			mHasUV = false;
		}
		if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
		{
			mAllByControlPoint = false;
		}
	}
	// 最终顶点个数到底是多少？
	// 如果只有一套UV，即UV映射方式是按实际顶点个数(FbxGeometryElement::eByControlPoint)，那么就是实际顶点个数。
	// 如果有多套UV，那么一个顶点在不同的多边形里，会对应不同的UV坐标，即UV映射方式是按多边形(eByPolygonVertex)，那么顶点个数是多边形数*3.
	int lPolygonVertexCount = mAllByControlPoint ? pMesh->GetControlPointsCount() : lPolygonCount * 3;
	// 创建数组存放所有顶点坐标。
	float* lVertices = new float[lPolygonVertexCount * 3];
	// 创建数组存放索引数据，数组长度=面数*3.
	unsigned short* lIndices = new unsigned short[lPolygonCount * 3];
	// 获取多套UV名字
	float* lUVs = NULL;
	FbxStringList lUVNames;
	pMesh->GetUVSetNames(lUVNames);
	const char* lUVName = NULL;
	if (mHasUV && lUVNames.GetCount())
	{
		// 创建数组存放UV数据
		lUVs = new float[lPolygonVertexCount * 2];
		// 暂时只使用第一套UV。
		lUVName = lUVNames[0];
	}
	// 实际顶点数据。
	const FbxVector4* lControlPoints = pMesh->GetControlPoints();
	// 遍历所有三角面，遍历每个面的三个顶点，解析顶点坐标、UV坐标数据。
	int lVertexCount = 0;
	for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
	{
		// 三角面，3个顶点
		for (int lVerticeIndex = 0; lVerticeIndex < 3; ++lVerticeIndex)
		{
			// 传入面索引，以及当前面的第几个顶点，获取顶点索引。
			const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
			if (lControlPointIndex >= 0) {
				// 因为设定一个顶点有多套UV，所以每个三角面与其他面相邻的共享的顶点，尽管实际上是同一个点(ControlPoint),因为有不同的UV，所以还是算不同的顶点。
				lIndices[lVertexCount] = static_cast<unsigned short>(lVertexCount);
				// 获取当前顶点索引对应的实际顶点。
				FbxVector4 lCurrentVertex = lControlPoints[lControlPointIndex];
				// 将顶点坐标从FbxVector4转为float数组
				lVertices[lVertexCount * 3] = static_cast<float>(lCurrentVertex[0]);
				lVertices[lVertexCount * 3 + 1] = static_cast<float>(lCurrentVertex[1]);
				lVertices[lVertexCount * 3 + 2] = static_cast<float>(lCurrentVertex[2]);
				if (mHasUV) {
					// 获取当前顶点在指定UV层的UV坐标，前面说过，一个顶点可能有多套UV。
					bool lUnmappedUV;
					FbxVector2 lCurrentUV;
					pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
					// 将UV坐标从FbxVector2转为float数组
					lUVs[lVertexCount * 2] = static_cast<float>(lCurrentUV[0]);
					lUVs[lVertexCount * 2 + 1] = static_cast<float>(lCurrentUV[1]);
				}
			}
			++lVertexCount;
		}
	}
	// 创建引擎Mesh文件，从FBX中解析数据填充到里面。
	skeletalMeshData->Vertices.resize(lVertexCount);
	skeletalMeshData->Indices.resize(lVertexCount);
	auto tc = pMesh->GetElementTangentCount();
	for (size_t i = 0; i < lVertexCount; i++)
	{
		skeletalMeshData->Vertices[i].Pos.x = lVertices[i * 3];
		skeletalMeshData->Vertices[i].Pos.y = lVertices[i * 3 + 1];
		skeletalMeshData->Vertices[i].Pos.z = lVertices[i * 3 + 2];
		skeletalMeshData->Vertices[i].TexC.x = lUVs[i * 2];
		skeletalMeshData->Vertices[i].TexC.y = lUVs[i * 2 + 1];
		//data[i].color_ = glm::vec4(1.0f);
		//data[i].uv_ = glm::vec2(lUVs[i * 2], lUVs[i * 2 + 1]);
	}

	for (size_t i = 0; i < lVertexCount; i++)
	{
		skeletalMeshData->Indices[i] = *lIndices++;
	}

	SkeletalSubset subset;
	subset.VertexStart = 0;
	subset.VertexCount = lVertexCount;
	subset.IndexStart = 0;
	subset.IndexCount = lVertexCount * 3;
	skeletalMeshData->Subsets.push_back(subset);

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
	VertexRelateBoneInfo* vertex_relate_bone_infos_ = new VertexRelateBoneInfo[lVertexCount];

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
		}
	}

	//上面记录了所有实际顶点的权重，下面要设置到逻辑顶点上。
	// 是否有UV数据？
	mAllByControlPoint = true;
	lUVMappingMode = FbxGeometryElement::eNone;
	if (mHasUV) {
		lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
		if (lUVMappingMode == FbxGeometryElement::eNone) {
			mHasUV = false;
		}
		if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint) {
			mAllByControlPoint = false;
		}
	}

	// 遍历所有三角面，遍历每个面的三个顶点，解析顶点坐标、UV坐标数据。
	for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex) {
		// 三角面，3个顶点
		for (int lVerticeIndex = 0; lVerticeIndex < 3; ++lVerticeIndex) {
			// 传入面索引，以及当前面的第几个顶点，获取顶点索引。
			const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
			if (lControlPointIndex >= 0) {
				VertexRelateBoneInfo vertex_relate_bone_info = vertex_relate_bone_infos_[lControlPointIndex];
				for (int i = 0; i < 4; ++i) {
					char bone_index = vertex_relate_bone_info.bone_index_[i];
					char weight = vertex_relate_bone_info.bone_weight_[i];
					skeletalMeshData->Vertices[lPolygonIndex * 3 + lVerticeIndex].BoneIndices[i] = bone_index < 0 ? 0 : bone_index;
					if (i < 3)
					{
						skeletalMeshData->Vertices[lPolygonIndex * 3 + lVerticeIndex].BoneWeights[i] = weight/100.f;
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
		meshFile << "DiffuseMap: " << "jacket_diff.dds" << std::endl;
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
		meshFile << i;
		if (i > 0 && (i + 1) % 3 == 0)
		{
			meshFile << std::endl;
		}
		else
		{
			meshFile << " ";
		}
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
			transform.Position = { (float)t.mData[0], (float)t.mData[1], (float)t.mData[2] };

			auto r = pNode->LclRotation.Get();
			transform.Rotation = { (float)r.mData[0], (float)r.mData[1], (float)r.mData[2] };

			auto s = pNode->LclScaling.Get();
			transform.Scale = { (float)s.mData[0], (float)s.mData[1], (float)s.mData[2] };

			skeletalMeshData->BoneOffsets.push_back(transform);

			s_SkeletalIndex++;
		}
	}

	// 递归解析子节点
	const int lChildCount = pNode->GetChildCount();

	auto parentIndex = s_SkeletalIndex;
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		ParseNode(pNode->GetChild(lChildIndex), skeletalMeshData, parentIndex);
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
		fbxScene->FillAnimStackNameArray(animStackNameArray);
		for (size_t i = 0; i < animStackNameArray.Size(); i++)
		{
			auto animBuffer = fbxScene->FindMember<FbxAnimStack>(animStackNameArray[i]->Buffer());

			// 获取动画片段的时间范围
			FbxTime mStart, mStop;
			FbxTakeInfo* lCurrentTakeInfo = fbxScene->GetTakeInfo(*(animStackNameArray[i]));
			if (lCurrentTakeInfo)
			{
				mStart = lCurrentTakeInfo->mLocalTimeSpan.GetStart();
				mStop = lCurrentTakeInfo->mLocalTimeSpan.GetStop();
			}

			AnimationClip animationClip;
			FbxTime::EMode lTimeMode = fbxScene->GetGlobalSettings().GetTimeMode();
			//animation.frame_per_second_ = fbxsdk::FbxTime::GetFrameRate(lTimeMode);

			// 每一帧的时间
			FbxTime mFrameTime;
			mFrameTime.SetTime(0, 0, 0, 1, 0, fbxScene->GetGlobalSettings().GetTimeMode());
			int lSkinIndex = 0;
			FbxSkin* lSkinDeformer = (FbxSkin*)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);

			animationClip.BoneAnimations.resize(lSkinDeformer->GetClusterCount());
			for (FbxTime pTime = mStart; pTime < mStop; pTime += mFrameTime)
			{
				// 首先获取当前节点的全局坐标
				FbxAMatrix lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
				FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;//相乘获得pNode在当前时间相对原点的坐标。

				//V_Array<Matrix4x4> one_frame_bone_matrix_vec;//一帧的所有骨骼变换矩阵
				auto count = lSkinDeformer->GetClusterCount();
				for (int lClusterIndex = 0; lClusterIndex < count; ++lClusterIndex)
				{
					Keyframe frame;
					frame.TimePos = (float)pTime.GetSecondDouble();

					// 获取骨骼的顶点组
					FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
					// 计算这个骨骼的形变，前面pNode是指计算到Mesh节点的形变，而这是是计算骨骼节点，后面会作用到顶点。
					FbxAMatrix lVertexTransformMatrix;
					ComputeClusterDeformation(lGlobalOffPosition, pMesh, lCluster, lVertexTransformMatrix, pTime);
					frame.BoneMatrix = FbxMatrixToGlmMat4(lVertexTransformMatrix);


					animationClip.BoneAnimations[lClusterIndex].Keyframes.push_back(frame);
				}

				//animation.frame_bones_matrix_vec_.push_back(one_frame_bone_matrix_vec);
			}//lClusterCount
			//animation.frame_count_ = animation.frame_bones_matrix_vec_.size();
			//animation.Write(fmt::format("../data/animation/fbx_extra_{}.skeleton_anim", animation.name_).c_str());

			skeletalMeshData->AnimClips[animBuffer->GetName()] = animationClip;
		}
	}
}