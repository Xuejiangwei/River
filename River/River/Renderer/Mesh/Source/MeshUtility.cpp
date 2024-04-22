#include "RiverPch.h"
#include "Renderer/Mesh/Header/MeshUtility.h"
#include "Renderer/MeshAnimation/Header/MeshAnimationData.h"
#include "Renderer/Header/Material.h"
#include "Renderer/Header/Texture.h"
#include "Renderer/DX12Renderer/Header/DX12DefaultConfig.h"
#include "Renderer/Header/AssetManager.h"

#include "fbxsdk/core/fbxmanager.h"
#include "fbxsdk.h"

#include <fstream>

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

void ParseMesh(const FbxMesh* pMesh)
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
	SkeletalMeshData data;
	data.Vertices.resize(lVertexCount);
	data.Indices.resize(lVertexCount);

	for (size_t i = 0; i < lVertexCount; i++)
	{
		data.Vertices[i].Pos.x = lVertices[i * 3];
		data.Vertices[i].Pos.y = lVertices[i * 3 + 1];
		data.Vertices[i].Pos.z = lVertices[i * 3 + 2];
		//data[i].color_ = glm::vec4(1.0f);
		//data[i].uv_ = glm::vec2(lUVs[i * 2], lUVs[i * 2 + 1]);
	}

	for (size_t i = 0; i < lVertexCount; i++)
	{
		data.Indices[i] = *lIndices++;
	}

	// 写入文件
	//mesh_file.Write(fmt::format("../data/model/fbx_extra_{}.mesh", mesh_file.head_.name_).c_str());
}

void ParseNode(FbxNode* pNode)
{
	// 获取节点属性
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
	if (lNodeAttribute) {
		// 节点是Mesh
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh) {
			FbxMesh* lMesh = pNode->GetMesh();
			if (lMesh && !lMesh->GetUserDataPtr()) {
				ParseMesh(lMesh);
			}
		}
	}
	// 递归解析子节点
	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex) {
		ParseNode(pNode->GetChild(lChildIndex));
	}
}

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
		//fbx
		static FbxManager* sdkManager = nullptr;
		static FbxScene* fbxScene = nullptr;
		static FbxImporter* fbxImporter = nullptr;

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

		int lFileFormat = -1;
		if (!sdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(path.c_str(), lFileFormat)) {
			// 未能识别文件格式
			return false;
		}

		// 初始化Importer，设置文件路径
		if (fbxImporter->Initialize(path.c_str(), lFileFormat) == false)
		{
			return false;
		}

		// 将FBX文件解析导入到Scene中
		bool lResult = fbxImporter->Import(fbxScene);
		if (!lResult)
		{
			LOG("Call to FbxImporter::Import() failed.Error reported: %s", fbxImporter->GetStatus().GetErrorString());
		}

		// 检查Scene完整性
		FbxStatus status;
		FbxArray<FbxString*> details;
		FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(fbxScene), &status, &details);
		bool lNotify = (!sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData) && details.GetCount() > 0) ||
			(fbxImporter->GetStatus().GetCode() != FbxStatus::eSuccess);
		//输出错误信息
		if (lNotify) {
			//return LogSceneCheckError(fbxImporter, details);
		}
		// 转换坐标系为右手坐标系。
		FbxAxisSystem SceneAxisSystem = fbxScene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
		if (SceneAxisSystem != OurAxisSystem) {
			OurAxisSystem.ConvertScene(fbxScene);
		}
		// 转换单元长度
		FbxSystemUnit SceneSystemUnit = fbxScene->GetGlobalSettings().GetSystemUnit();
		if (SceneSystemUnit.GetScaleFactor() != 1.0) {
			// 例子中用的是厘米，所以这里也要转换
			FbxSystemUnit::cm.ConvertScene(fbxScene);
		}
		// 转换曲面到三角形
		FbxGeometryConverter lGeomConverter(sdkManager);
		try {
			lGeomConverter.Triangulate(fbxScene, /*replace*/true);
		}
		catch (std::runtime_error) {
			LOG("Scene integrity verification failed.\n");
			return false;
		}
		// 递归解析节点
		ParseNode(fbxScene->GetRootNode());
		LOG("extra mesh success");
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