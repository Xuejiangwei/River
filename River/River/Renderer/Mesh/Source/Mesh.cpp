#include "RiverPch.h"
#include "Renderer/Mesh/Header/StaticMesh.h"

Mesh::Mesh(String& name, String& path)
	: m_Name(River::Move(name)), m_Path(River::Move(path))
{
}

Mesh::~Mesh()
{
}
