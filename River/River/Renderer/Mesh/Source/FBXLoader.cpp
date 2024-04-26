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
	char bone_index_[4];//����������һ���������128������char���С�
	char bone_weight_[4];//����Ȩ�أ�Ȩ�ز��ᳬ��100��������char���;Ϳ��ԡ�
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
		// δ��ʶ���ļ���ʽ
		return false;
	}

	// ��ʼ��Importer�������ļ�·��
	if (!fbxImporter->Initialize(path.c_str(), lFileFormat))
	{
		return false;
	}

	// ��FBX�ļ��������뵽Scene��
	if (!fbxImporter->Import(fbxScene))
	{
		LOG("Call to FbxImporter::Import() failed.Error reported: %s", fbxImporter->GetStatus().GetErrorString());
		return false;
	}

	// ���Scene������
	FbxStatus status;
	FbxArray<FbxString*> details;
	FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(fbxScene), &status, &details);
	//���������Ϣ
	if ((!sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData) && details.GetCount() > 0) ||
		(fbxImporter->GetStatus().GetCode() != FbxStatus::eSuccess))
	{
		return false;
	}

	// ת������ϵΪ��������ϵ��
	FbxAxisSystem SceneAxisSystem = fbxScene->GetGlobalSettings().GetAxisSystem();
	FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
	if (SceneAxisSystem != OurAxisSystem)
	{
		OurAxisSystem.ConvertScene(fbxScene);
	}

	// ת����Ԫ����
	FbxSystemUnit SceneSystemUnit = fbxScene->GetGlobalSettings().GetSystemUnit();
	if (SceneSystemUnit.GetScaleFactor() != 1.0)
	{
		// �������õ������ף���������ҲҪת��
		FbxSystemUnit::cm.ConvertScene(fbxScene);
	}

	// ת�����浽������
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

	// �ݹ�����ڵ�
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
	// ��ȡMesh����θ���������Ϸ��˵����������������
	const int lPolygonCount = pMesh->GetPolygonCount();
	// �Ƿ���UV���ݣ�
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
	// ���ն�����������Ƕ��٣�
	// ���ֻ��һ��UV����UVӳ�䷽ʽ�ǰ�ʵ�ʶ������(FbxGeometryElement::eByControlPoint)����ô����ʵ�ʶ��������
	// ����ж���UV����ôһ�������ڲ�ͬ�Ķ��������Ӧ��ͬ��UV���꣬��UVӳ�䷽ʽ�ǰ������(eByPolygonVertex)����ô��������Ƕ������*3.
	int lPolygonVertexCount = mAllByControlPoint ? pMesh->GetControlPointsCount() : lPolygonCount * 3;
	// �������������ж������ꡣ
	float* lVertices = new float[lPolygonVertexCount * 3];
	// �����������������ݣ����鳤��=����*3.
	unsigned short* lIndices = new unsigned short[lPolygonCount * 3];
	// ��ȡ����UV����
	float* lUVs = NULL;
	FbxStringList lUVNames;
	pMesh->GetUVSetNames(lUVNames);
	const char* lUVName = NULL;
	if (mHasUV && lUVNames.GetCount())
	{
		// ����������UV����
		lUVs = new float[lPolygonVertexCount * 2];
		// ��ʱֻʹ�õ�һ��UV��
		lUVName = lUVNames[0];
	}
	// ʵ�ʶ������ݡ�
	const FbxVector4* lControlPoints = pMesh->GetControlPoints();
	// �������������棬����ÿ������������㣬�����������ꡢUV�������ݡ�
	int lVertexCount = 0;
	for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex)
	{
		// �����棬3������
		for (int lVerticeIndex = 0; lVerticeIndex < 3; ++lVerticeIndex)
		{
			// �������������Լ���ǰ��ĵڼ������㣬��ȡ����������
			const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);
			if (lControlPointIndex >= 0) {
				// ��Ϊ�趨һ�������ж���UV������ÿ�������������������ڵĹ���Ķ��㣬����ʵ������ͬһ����(ControlPoint),��Ϊ�в�ͬ��UV�����Ի����㲻ͬ�Ķ��㡣
				lIndices[lVertexCount] = static_cast<unsigned short>(lVertexCount);
				// ��ȡ��ǰ����������Ӧ��ʵ�ʶ��㡣
				FbxVector4 lCurrentVertex = lControlPoints[lControlPointIndex];
				// �����������FbxVector4תΪfloat����
				lVertices[lVertexCount * 3] = static_cast<float>(lCurrentVertex[0]);
				lVertices[lVertexCount * 3 + 1] = static_cast<float>(lCurrentVertex[1]);
				lVertices[lVertexCount * 3 + 2] = static_cast<float>(lCurrentVertex[2]);
				if (mHasUV) {
					// ��ȡ��ǰ������ָ��UV���UV���꣬ǰ��˵����һ����������ж���UV��
					bool lUnmappedUV;
					FbxVector2 lCurrentUV;
					pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);
					// ��UV�����FbxVector2תΪfloat����
					lUVs[lVertexCount * 2] = static_cast<float>(lCurrentUV[0]);
					lUVs[lVertexCount * 2 + 1] = static_cast<float>(lCurrentUV[1]);
				}
			}
			++lVertexCount;
		}
	}
	// ��������Mesh�ļ�����FBX�н���������䵽���档
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

	// ��ȡ��Ƥ�޸���
	int lSkinIndex = 0;
	FbxSkin* lSkinDeformer = (FbxSkin*)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
	// ��ȡ��Ƥ�޸����ϵĶ�����������һ����˵���ǹ����������󶨵�ʱ��һ������һ��������Ϊһ�������顣
	int lClusterCount = lSkinDeformer->GetClusterCount();

	// ��������
	VertexRelateBoneInfo* vertex_relate_bone_infos_ = new VertexRelateBoneInfo[lVertexCount];

	//skeletalMeshData->BoneHierarchy.resize(lClusterCount);
	for (int lClusterIndex = 0; lClusterIndex < lClusterCount; ++lClusterIndex)
	{
		// ��ȡ�����Ķ�����
		/*FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
		
		animation.bone_name_vec_.push_back(lCluster->GetName());*/

		// ��ȡ�����Ķ�����
		FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
		// ��ȡ���������Ӱ��Ķ�����������
		int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
		for (int k = 0; k < lVertexIndexCount; ++k)
		{
			//�õ���������
			int lIndex = lCluster->GetControlPointIndices()[k];
			//�õ�������ж���������Ȩ��
			auto lWeight = lCluster->GetControlPointWeights()[k];
			vertex_relate_bone_infos_[lIndex].Push(lClusterIndex, (int)(lWeight * 100));
		}
	}

	//�����¼������ʵ�ʶ����Ȩ�أ�����Ҫ���õ��߼������ϡ�
	// �Ƿ���UV���ݣ�
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

	// �������������棬����ÿ������������㣬�����������ꡢUV�������ݡ�
	for (int lPolygonIndex = 0; lPolygonIndex < lPolygonCount; ++lPolygonIndex) {
		// �����棬3������
		for (int lVerticeIndex = 0; lVerticeIndex < 3; ++lVerticeIndex) {
			// �������������Լ���ǰ��ĵڼ������㣬��ȡ����������
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
	// д���ļ�
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
	// ��ȡ�ڵ�����
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
	if (lNodeAttribute)
	{
		// �ڵ���Mesh
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

	// �ݹ�����ӽڵ�
	const int lChildCount = pNode->GetChildCount();

	auto parentIndex = s_SkeletalIndex;
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		ParseNode(pNode->GetChild(lChildIndex), skeletalMeshData, parentIndex);
	}
}

/// ��Mesh�ڵ��ȡ��Ƥ�޸�������ȡ����Cluster������֡������ȡÿһ֡ÿ��Cluster���ƶ���ת�������ݡ�
/// \param pNode Mesh�ڵ�
void ParseAniamtion(FbxNode* pNode, SkeletalMeshData* skeletalMeshData)
{
	auto name = pNode->GetName();
	LOG("node name: %s", name);
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
	if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
	{
		// ��ȡpNode�����ê���offset
		const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
		FbxAMatrix lGeometryOffset = FbxAMatrix(lT, lR, lS);
		LOG(" node translation: (%f,%f,%f,%f)", lT[0], lT[1], lT[2], lT[3]);
		LOG(" node rotation: (%f,%f,%f,%f)", lR[0], lR[1], lR[2], lR[3]);
		LOG(" node scaling: (%f,%f,%f,%f)", lS[0], lS[1], lS[2], lS[3]);

		FbxMesh* pMesh = pNode->GetMesh();
		// ��ȡ��Ƥ������һ����˵һ��Mesh��Ӧһ����Ƥ�޸�����
//            int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
			//Engine::Animation animation;
		FbxArray<FbxString*>animStackNameArray;
		fbxScene->FillAnimStackNameArray(animStackNameArray);
		for (size_t i = 0; i < animStackNameArray.Size(); i++)
		{
			auto animBuffer = fbxScene->FindMember<FbxAnimStack>(animStackNameArray[i]->Buffer());

			// ��ȡ����Ƭ�ε�ʱ�䷶Χ
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

			// ÿһ֡��ʱ��
			FbxTime mFrameTime;
			mFrameTime.SetTime(0, 0, 0, 1, 0, fbxScene->GetGlobalSettings().GetTimeMode());
			int lSkinIndex = 0;
			FbxSkin* lSkinDeformer = (FbxSkin*)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);

			animationClip.BoneAnimations.resize(lSkinDeformer->GetClusterCount());
			for (FbxTime pTime = mStart; pTime < mStop; pTime += mFrameTime)
			{
				// ���Ȼ�ȡ��ǰ�ڵ��ȫ������
				FbxAMatrix lGlobalPosition = pNode->EvaluateGlobalTransform(pTime);
				FbxAMatrix lGlobalOffPosition = lGlobalPosition * lGeometryOffset;//��˻��pNode�ڵ�ǰʱ�����ԭ������ꡣ

				//V_Array<Matrix4x4> one_frame_bone_matrix_vec;//һ֡�����й����任����
				auto count = lSkinDeformer->GetClusterCount();
				for (int lClusterIndex = 0; lClusterIndex < count; ++lClusterIndex)
				{
					Keyframe frame;
					frame.TimePos = (float)pTime.GetSecondDouble();

					// ��ȡ�����Ķ�����
					FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
					// ��������������α䣬ǰ��pNode��ָ���㵽Mesh�ڵ���α䣬�������Ǽ�������ڵ㣬��������õ����㡣
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