#pragma once
#include "RiverHead.h"
#include "Mesh.h"

class SkeletalMesh : public Mesh
{
public:
	SkeletalMesh(String&& name, String&& path);

	virtual ~SkeletalMesh() override;
};