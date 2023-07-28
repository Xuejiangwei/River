//***************************************************************************************
// GeometryGenerator.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Defines a static class for procedurally generating the geometry of 
// common mathematical objects.
//
// All triangles are generated "outward" facing.  If you want "inward" 
// facing triangles (for example, if you want to place the camera inside
// a sphere to simulate a sky), you will need to:
//   1. Change the Direct3D cull mode or manually reverse the winding order.
//   2. Invert the normal.
//   3. Update the texture coordinates and tangent vectors.
//***************************************************************************************

#pragma once

#include "RiverHead.h"
#include <cstdint>
#include <wrl.h>
#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>

struct SubmeshGeometry
{
	uint32_t IndexCount = 0;
	uint32_t StartIndexLocation = 0;
	int BaseVertexLocation = 0;

	DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
	std::string Name;

	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;
	Unique<class VertexBuffer> VertexBuffer;
	Unique<class IndexBuffer> IndexBufer;

	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;
};

class DX12GeometryGenerator
{
public:

	struct MeshVertex
	{
		MeshVertex() {}
		MeshVertex(
			const DirectX::XMFLOAT3& p,
			const DirectX::XMFLOAT3& n,
			const DirectX::XMFLOAT3& t,
			const DirectX::XMFLOAT2& uv) :
			Position(p),
			Normal(n),
			TangentU(t),
			TexC(uv) {}
		MeshVertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			Position(px, py, pz),
			Normal(nx, ny, nz),
			TangentU(tx, ty, tz),
			TexC(u, v) {}

		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 TangentU;
		DirectX::XMFLOAT2 TexC;
	};

	struct MeshData
	{
		std::vector<MeshVertex> Vertices;
		std::vector<uint32_t> Indices32;
		DirectX::BoundingBox BB;
	};

public:
	DX12GeometryGenerator();

	~DX12GeometryGenerator();

	void Initialize();

	Unique<MeshGeometry>& LoadMeshByFile(const char* filePath, const char* name);

	Unique<MeshGeometry>& GetMesh(const char* name);

	static Unique<DX12GeometryGenerator>& Get();

	static const char* BoxName;
	static const char* SphereName;
	static const char* GeosphereName;
	static const char* CylinderName;
	static const char* GridName;
	static const char* QuadName;

private:
	///<summary>
	/// Creates a box centered at the origin with the given dimensions, where each
	/// face has m rows and n columns of vertices.
	///</summary>
	Unique<MeshGeometry> CreateBox(float width, float height, float depth, uint32_t numSubdivisions);

	///<summary>
	/// Creates a sphere centered at the origin with the given radius.  The
	/// slices and stacks parameters control the degree of tessellation.
	///</summary>
	Unique<MeshGeometry> CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount);

	///<summary>
	/// Creates a geosphere centered at the origin with the given radius.  The
	/// depth controls the level of tessellation.
	///</summary>
	Unique<MeshGeometry> CreateGeosphere(float radius, uint32_t numSubdivisions);

	///<summary>
	/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
	/// The bottom and top radius can vary to form various cone shapes rather than true
	// cylinders.  The slices and stacks parameters control the degree of tessellation.
	///</summary>
	Unique<MeshGeometry> CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount);

	///<summary>
	/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
	/// at the origin with the specified width and depth.
	///</summary>
	Unique<MeshGeometry> CreateGrid(float width, float depth, uint32_t m, uint32_t n);

	///<summary>
	/// Creates a quad aligned with the screen.  This is useful for postprocessing and screen effects.
	///</summary>
	Unique<MeshGeometry> CreateQuad(float x, float y, float w, float h, float depth);

	Unique<MeshGeometry> CreateMeshGeometry(const char* name, void* vertices, void* indices, unsigned int vertexCount, unsigned int indiceCount,
		unsigned int verticesByteSize, unsigned int indicesByteSize);

private:
	void Subdivide(MeshData& meshData);
	MeshVertex MidPoint(const MeshVertex& v0, const MeshVertex& v1);
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount, MeshData& meshData);

private:
	HashMap<String, Unique<MeshGeometry>> m_Geometries;

	static Unique<DX12GeometryGenerator> s_Instance;
};

