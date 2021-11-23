#pragma once

#include "Transform.hpp"
#include "Mesh.hpp"

class RenderObject
{
public:
	RenderObject(ID3D11Device* const pDevice) : transform(pDevice)
	{

	}
	~RenderObject() = default;
	ROF_DELETE(RenderObject);

private:
	Transform transform;
	Mesh mesh;
};